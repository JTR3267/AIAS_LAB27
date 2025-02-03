#!/usr/bin/env python3

# Copyright 2023-2024 Playlab/ACAL
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from typing import Dict, List, Tuple, Optional
import sys
import os
import re
import subprocess

import click

from acalsim_template import PROJ_ARR

BUILD_DIR: str = "build"
REGRESSION_LOG_DIR: str = "build/regression"
BUILD_LOG_FILENAME: str = "build.log"
EXEC_LOG_FILENAME: str = "exec.log"
COMPILE_TYPE_LIST: List[str] = ["Debug", "Release", "GTest"]
IGNORED_COMPILE_TYPE: List[str] = [COMPILE_TYPE_LIST[2]]

# Regular expressions
COLOR_REGEX: re.Pattern = re.compile(r"\033\[[0-9;]+m")
TIMETICK_REGEX: re.Pattern = re.compile(r"Tick=(\d+) Info: \[.+\] Simulation complete\.")

# Determined by task_analysis() dynamically
MAX_COMPILEMODE_LENGTH: int = 0
MAX_LINE_WIDTH: int = 0


class ValidationError(Exception):

	def __init__(self, message):
		super().__init__(message)


def move_to_root_dir() -> None:
	os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))


def remove_existing_logs() -> None:
	if os.path.isdir(REGRESSION_LOG_DIR):
		subprocess.run(["rm", "-r", REGRESSION_LOG_DIR], check=True)


def task_analysis() -> None:
	global MAX_COMPILEMODE_LENGTH
	global MAX_LINE_WIDTH
	MAX_COMPILEMODE_LENGTH = max(len(x["compile-mode"]) for x in PROJ_ARR) + 2
	MAX_LINE_WIDTH = max(max(len(x["name"]), len(x["log-name"])) for x in PROJ_ARR) + 24


def exec_cmd(
    cmd: List[str],
    log: str,
    file_mode: str = "w",
    timeout: Optional[float] = None,
    verify_sim_tick_value: Optional[int] = None,
    verify_sim_tick_range: Optional[Tuple[int, int]] = None
) -> None:
	try:
		if timeout != None:
			proc = subprocess.run(
			    cmd,
			    timeout=timeout,
			    stdout=subprocess.PIPE,
			    stderr=subprocess.STDOUT,
			    text=True,
			    check=True
			)
		else:
			proc = subprocess.run(
			    cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, check=True
			)

		sim_complete_tick: Optional[int] = None

		with open(log, mode=file_mode) as f_log:
			for line in proc.stdout.splitlines():
				line = COLOR_REGEX.sub("", line)
				m = re.search(TIMETICK_REGEX, line)
				sim_complete_tick = int(m.group(1)) if m else sim_complete_tick
				f_log.write(f"{line}\n")

		# Check if sim_complete_tick is None
		if (verify_sim_tick_value or verify_sim_tick_range) and not sim_complete_tick:
			raise ValidationError("The simulation did not print the complete time.")

		# Validate the complete time tick with verify_sim_tick_value
		if verify_sim_tick_value and not (verify_sim_tick_value == sim_complete_tick):
			raise ValidationError(
			    f"The simulation did not complete at the expected tick. Received tick: {sim_complete_tick}"
			)

		# Validate the complete time tick with verify_sim_tick_range
		if verify_sim_tick_range and not (
		    verify_sim_tick_range[0] <= sim_complete_tick <= verify_sim_tick_range[1]
		):
			raise ValidationError(
			    f"The simulation did not complete at the expected tick range. Received tick: {sim_complete_tick}"
			)

	except subprocess.CalledProcessError as e:
		with open(log, mode=file_mode) as f_log:
			for line in e.stdout.splitlines() if e.stdout else []:
				line = COLOR_REGEX.sub("", line)
				f_log.write(f"{line}\n")
			for line in e.stderr.splitlines() if e.stderr else []:
				line = COLOR_REGEX.sub("", line)
				f_log.write(f"{line}\n")
			f_log.write(f"The command (or program) exited with return code {e.returncode}.")
		raise e

	except subprocess.TimeoutExpired as e:
		with open(log, mode=file_mode) as f_log:
			f_log.write(
			    "The outputs are not captured because a program that times out might generate an excessive amount of logs.\n"
			)
		raise e


def print_result(
    title: str,
    mode: str,
    result: bool,
    stage: str = "",
    exception: Optional[Exception] = None
) -> None:
	COLOR_CODE: Dict[str, str] = {"RED": '\033[0;31m', "GREEN": '\033[0;32m', "NC": '\033[0m'}
	INTERNAL_ERROR_MSG: str = "Regression Error"

	result_str: str = ""

	if result:
		result_str = "Passed"
	elif type(exception) == subprocess.CalledProcessError:
		result_str = stage + " " + "Failed" if len(stage) > 0 else "Failed"
	elif type(exception) == subprocess.TimeoutExpired:
		result_str = stage + " " + "Timeout" if len(stage) > 0 else "Timeout"
	elif type(exception) == ValidationError:
		result_str = "Incorrect Result"
	else:
		result_str = INTERNAL_ERROR_MSG

	colored_result_str: str = (
	    COLOR_CODE["GREEN"] + result_str if result else COLOR_CODE["RED"] + result_str
	) + COLOR_CODE["NC"]

	print(
	    f"[{mode.center(MAX_COMPILEMODE_LENGTH)}] {title} {'.'*(MAX_LINE_WIDTH - len(title) - len(result_str))} {colored_result_str}"
	)

	if result_str == INTERNAL_ERROR_MSG and exception:
		print(
		    " " * (MAX_COMPILEMODE_LENGTH + 3) + f"{exception.__class__.__name__}: {str(exception)}"
		)


def exec_proj(
    name: str,
    log_name: str,
    src_dirname: str,
    exec_args: List[str],
    compile_mode: str,
    pre_steps: List[str],
    post_steps: List[str],
    timeout: Optional[int],
    expected_tick_value: Optional[int] = None,
    expected_tick_range: Optional[Tuple[int, int]] = None
) -> bool:
	# Variables
	build_dir: str = os.path.join(BUILD_DIR, compile_mode.lower())
	log_dir: str = os.path.join(REGRESSION_LOG_DIR, compile_mode.lower(), src_dirname)
	build_log: str = os.path.join(log_dir, BUILD_LOG_FILENAME)
	exec_log: str = os.path.join(log_dir, f"{log_name}-{EXEC_LOG_FILENAME}")

	if compile_mode not in COMPILE_TYPE_LIST:
		raise RuntimeError(f"The compile mode '{compile_mode}' is unsupported.")

	if compile_mode == "GTest":
		src_dirname = "g" + src_dirname

	try:
		# Create a log folder
		subprocess.run(["mkdir", "-p", log_dir], check=True)

		# Configure CMake
		exec_cmd(
		    cmd=[
		        "cmake", "-B", build_dir, f"-DCMAKE_BUILD_TYPE={compile_mode}"
		        if compile_mode not in IGNORED_COMPILE_TYPE else "", "-DNO_LOGS=OFF"
		    ],
		    log=build_log,
		    file_mode="w"
		)

		# Compile the executable
		exec_cmd(
		    cmd=["cmake", "--build", build_dir, f"-j{os.cpu_count()}", "--target", src_dirname],
		    log=build_log,
		    file_mode="a"
		)

	except Exception as e:
		print_result(title=name, mode=compile_mode, result=False, stage="Compile", exception=e)
		return False

	try:
		# Run the executable
		with open(exec_log, mode="w", encoding="utf-8") as file:
			file.write("======= Pre-steps =======\n")
		for cmd in pre_steps:
			exec_cmd(cmd=cmd, log=exec_log, file_mode="a")

		with open(exec_log, mode="a", encoding="utf-8") as file:
			file.write("\n======= Simulation =======\n")
		exec_cmd(
		    cmd=[os.path.join(build_dir, src_dirname)] + exec_args,
		    log=exec_log,
		    file_mode="a",
		    timeout=timeout,
		    verify_sim_tick_value=expected_tick_value,
		    verify_sim_tick_range=expected_tick_range
		)

		with open(exec_log, mode="a", encoding="utf-8") as file:
			file.write("\n======= Post-steps =======\n")
		for cmd in post_steps:
			exec_cmd(cmd=cmd, log=exec_log, file_mode="a")

		print_result(title=name, mode=compile_mode, result=True)

	except Exception as e:
		print_result(title=name, mode=compile_mode, result=False, stage="Execute", exception=e)
		return False

	return True


def test_projects(timeout: Optional[int]) -> None:
	success: bool = True

	for proj in PROJ_ARR:
		success = exec_proj(
		    name=proj["name"],
		    log_name=proj["log-name"],
		    src_dirname=proj["src-subdir"],
		    exec_args=proj["exec-args"],
		    compile_mode=proj["compile-mode"],
		    pre_steps=proj["pre-steps"],
		    post_steps=proj["post-steps"],
		    timeout=timeout,
		    expected_tick_value=proj["total-tick"] if "total-tick" in proj else None,
		    expected_tick_range=tuple(proj["total-tick-range"])
		    if "total-tick-range" in proj else None
		) & success

	if not success:
		exit(1)


@click.command()
@click.option(
    '--timeout',
    help='Time limitation in seconds for executing each test example.',
    default=30,
    required=False,
    type=int
)
def main(timeout: Optional[int]) -> None:
	try:
		move_to_root_dir()
		remove_existing_logs()
		task_analysis()
		test_projects(timeout=timeout)
	except KeyboardInterrupt as e:
		print("\n" + "Regression test is terminated.", file=sys.stderr)
		exit(1)


if __name__ == '__main__':
	main()
