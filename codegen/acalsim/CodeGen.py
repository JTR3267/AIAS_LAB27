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

from .SimTop import SimTop
from .SimBase import SimBase
from .SimModule import SimModule
from .BaseGen import ProjectGen, AppGen
import os
from typing import Union
from .util import (
    replace_define_macro,
    replace_inc_self_path,
)


class CodeGen(ProjectGen, AppGen):

	def __init__(
	    self,
	    instance: Union[SimTop, SimBase, SimModule],
	    genProj: bool = False,
	    projectName: str = ""
	):
		# Initialize all attributes to None by default
		self.simtop: SimTop = None
		self.simulator: SimBase = None
		self.simmodule: SimModule = None

		# Assign based on the type of the instance
		if isinstance(instance, SimTop):
			self.simtop = instance
			self.simtop.codegen()
			self.create_app()
		elif isinstance(instance, SimBase):
			self.simulator = instance
			self.simulator.codegen()
		elif isinstance(instance, SimModule):
			self.simmodule = instance
			self.simmodule.codegen()
		else:
			raise ValueError(
			    f"Unsupported instance type: {type(instance).__name__}. "
			    "Expected SimTop, SimBase, or SimModule."
			)
		if genProj:
			self.create_proj(projectName)

	def create_proj(self, project_name: str):
		self.create_proj_cmake(project_name)
		self.create_proj_library(project_name)

	def create_app(self):
		self.create_app_main(self.simtop)
		self.create_app_cmake(self.simtop.app, self.simtop.systemc)
		self.create_app_library_make(self.simtop.app, self.simtop.systemc)
