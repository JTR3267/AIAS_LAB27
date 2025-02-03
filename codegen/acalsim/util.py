# Copyright 2023-2025 Playlab/ACAL
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

import os
from typing import Callable


def create(tpl_path: str, dst_path: str, func: Callable[[str], str] = None):
	"""
    General function for creating files from templates.

    Args:
        tpl_path (str): Path to the template file.
        dst_path (str): Path to the destination file.
        func (Callable[[str], str]): Custom processing function to modify the template content.
                                     It takes the template content as input and returns the processed content.
    """
	# Read the template file
	with open(tpl_path, 'r') as f:
		content = f.read()

	#################################
	# Apply Custom Processing Logic #
	#################################
	content = func(content)

	# Ensure the destination directory exists
	os.makedirs(os.path.dirname(dst_path), exist_ok=True)

	# Write the processed content to the destination file
	with open(dst_path, 'w') as f:
		f.write(content)


def chain_lambdas(*funcs):
	"""Combine multiple lambda functions into a single function."""
	return lambda content: content if not funcs else chain_lambdas(*funcs[1 :])(funcs[0](content))


def replace_define_macro(content: str, file_path: str) -> str:
	file_path = file_path.replace('/', '_').replace('\\', '_').replace('.', '_').upper() + "_"
	return content.replace("{{util.define_macro}}", f"{file_path}")


def replace_class_name(content: str, class_name: str) -> str:
	return content.replace("{{util.class_name}}", class_name)


def replace_namespace_name(content: str, status: bool, namespace: str = "") -> str:

	head = f"\nnamespace {namespace} " + "{\n" if not status else ""
	footer = "\n}" + f"  // namespace {namespace}\n" if not status else ""

	content = content.replace("\n{{util.namespace_name.head}}", head)
	content = content.replace("\n{{util.namespace_name.footer}}", footer)

	return content


def replace_inc_path(content: str, self_path: str) -> str:
	return content.replace("{{util.inc_path}}", self_path)


def replace_inc_self_path(content: str, self_path: str) -> str:
	return content.replace("{{util.inc_self_path}}", self_path)


def replace_project_name(content, name):
	return content.replace("{{util.project_name}}", name)


def replace_app_lib_name(content, name):
	return content.replace("{{util.app_lib_name}}", name)
