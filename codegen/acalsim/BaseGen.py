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
from .SimTop import SimTop

from .util import (
    create, chain_lambdas, replace_project_name, replace_define_macro, replace_inc_self_path,
    replace_inc_path, replace_class_name, replace_app_lib_name
)


class ProjectGen:

	def create_proj_library(self, project_name: str):
		self.create_proj_library_cmake()
		self.create_proj_library_hh(project_name)
		self.create_proj_library_cc(project_name)

	def create_proj_cmake(self, project_name: str):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cmake", "Project.cmake")
		dst_path = os.path.join(".", "CMakeLists.txt")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_path,
		    func=chain_lambdas(lambda content: replace_project_name(content, project_name))
		)

	def create_proj_library_cmake(self):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cmake", "ProjectLibs.cmake")
		dst_path = os.path.join("libs", "CMakeLists.txt")

		create(tpl_path=tpl_path, dst_path=dst_path, func=chain_lambdas())

	def create_proj_library_hh(self, project_name: str):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TInclude.hh")
		dst_path = os.path.join("include", f"{project_name}.hh")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_path,
		    func=chain_lambdas(lambda content: replace_define_macro(content, dst_path))
		)

	def create_proj_library_cc(self, project_name: str):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TInclude.cc")

		dst_hh_path = os.path.join(f"{project_name}.hh")
		dst_cc_path = os.path.join("libs", f"{project_name}.cc")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_cc_path,
		    func=chain_lambdas(lambda content: replace_inc_self_path(content, dst_hh_path))
		)


class AppGen:

	def create_app_main(self, simtop: SimTop):
		if simtop.systemc:
			raise NotImplementedError(
			    "SystemC simulator is not integrated into code generation yet. "
			    "Please complete the integration before adding SystemC simulators."
			)
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TCCMain.cc")
		dst_path = os.path.join("src", simtop.app, "main.cc")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_path,
		    func=chain_lambdas(
		        lambda content: replace_inc_path(content, simtop.getIncPath(fullPath=False)),
		        lambda content: replace_class_name(content, simtop.getName(namespace=True))
		    )
		)

	def create_app_cmake(self, app: str, systemc: bool = False):
		if systemc:
			raise NotImplementedError(
			    "SystemC simulator is not integrated into code generation yet. "
			    "Please complete the integration before adding SystemC simulators."
			)
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cmake", "App.cmake")
		dst_path = os.path.join("src", app, "CMakeLists.txt")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_path,
		    func=chain_lambdas(lambda content: replace_app_lib_name(content, app))
		)

	def create_app_library_make(self, app: str, systemc: bool = False):
		if systemc:
			raise NotImplementedError(
			    "SystemC simulator is not integrated into code generation yet. "
			    "Please complete the integration before adding SystemC simulators."
			)
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cmake", "AppLibs.cmake")
		dst_path = os.path.join("src", app, "libs", "CMakeLists.txt")

		create(
		    tpl_path=tpl_path,
		    dst_path=dst_path,
		    func=chain_lambdas(lambda content: replace_app_lib_name(content, app))
		)
