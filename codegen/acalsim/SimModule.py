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

from .SimPort import SimPortManager
from .util import (
    create,
    chain_lambdas,
    replace_define_macro,
    replace_class_name,
    replace_namespace_name,
    replace_inc_self_path,
)
import os


class SimModule(SimPortManager):
	"""
    SimModule: A foundational simulation class for ACALSim library.
    Provides basic methods for setting up, running, and analyzing simulations.
    """

	def __init__(self, name: str, shared: bool, group: str, app: str = ""):
		"""
        Initialize the simulation base class.

        Args:
            name (str): Name of the simulation instance.
            config (dict): Configuration dictionary for the simulation.
        """
		super().__init__()
		self.name = name
		self.shared = shared
		self.group = group
		self.app = app

	def getFolderPath(self, type: str, fullPath: bool = True) -> str:

		if fullPath:
			if not self.shared:
				return os.path.join("src", self.app, type, self.group)
			else:
				return os.path.join(type, self.group)
		else:
			return os.path.join(self.group)

	def getIncPath(self, fullPath: bool = True) -> str:
		return os.path.join(self.getFolderPath("include", fullPath), f"{self.name}.hh")

	def getLibPath(self, fullPath: bool = True) -> str:
		return os.path.join(self.getFolderPath("libs", fullPath), f"{self.name}.cc")

	def getName(self, namespace: bool = False) -> str:
		return f"{self.group}::{self.name}" if self.group and namespace else self.name

	def codegen(self):
		if not self.shared and self.app == "":
			raise ValueError("The 'app' attribute cannot be an empty string.")
		self._create_hh()
		self._create_cc()

	def _create_hh(self):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TCCSimModule.hh")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getIncPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_define_macro(content, self.getIncPath(fullPath=True)),
		        lambda content: replace_class_name(content, self.name),
		        lambda content: replace_namespace_name(content, self.group == "", self.group)
		    )
		)

	def _create_cc(self):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TCCSimModule.cc")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getLibPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_class_name(content, self.name),
		        lambda content: replace_inc_self_path(content, self.getIncPath(fullPath=False)),
		        lambda content: replace_namespace_name(content, self.group == "", self.group),
		        lambda content: self.replace_simport(content)
		    )
		)
