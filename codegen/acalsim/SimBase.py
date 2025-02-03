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

from .SimModule import SimModule
from .SimPort import SimPortManager, replace_simport_connection
from .SimChannel import SimChannelManager
import os

from .util import (
    create,
    chain_lambdas,
    replace_define_macro,
    replace_class_name,
    replace_namespace_name,
    replace_inc_self_path,
)


class SimBase(SimPortManager, SimChannelManager):
	"""
	SimBase: A foundational simulation class for ACALSim library.
	Provides basic methods for setting up, running, and analyzing simulations.
	"""

	def __init__(self, name: str, shared: bool, group: str, systemc: bool = False, app: str = ""):
		"""
		Initialize the simulation base class.

		Args:
		name (str): Name of the simulation instance.
		shared (bool): Indicates if the simulation resources are shared.
		"""
		SimPortManager.__init__(self)
		SimChannelManager.__init__(self)
		self.name: str = name
		self.group: str = group
		self.shared: bool = shared
		self.systemc: bool = systemc
		self.app: str = app
		self.modules: dict[str, SimModule] = {}
		self.linked_mod_simport_list: list[dict] = []
		self.m_channel_ports: list[str] = []
		self.s_channel_ports: list[str] = []

	def addModule(self, module: SimModule):
		"""
		Add a module to the simulation.

		Args:
		module (SimModule): An instance of a simulation module to add.
		"""
		if module.name in self.modules:
			raise ValueError(f"Module with name '{module.name}' already exists.")
		self.modules[module.name] = module

		if not module.shared: module.app = self.app

		print(f"Module '{module.name}' added to simulator '{self.name}'.")

	def connectSimPort(
	    self,
	    master: SimModule,
	    slave: SimModule,
	    m_port_name: str,
	    s_port_name: str,
	    req_queue_size: int = 1
	):
		"""
		Connect a simulator simport between a master and a slave module.

		Args:
		master (SimModule): The master module.
		slave (SimModule): The slave module.
		m_port_name (str): The name of the master port.
		s_port_name (str): The name of the slave port.
		"""
		if master.name not in self.modules or slave.name not in self.modules:
			raise ValueError("Both master and slave modules must be added to the simulation first.")

		# Ensure the ports exist in the modules
		if not master._hasMasterSimPort(m_port_name):
			master._addMasterSimPort(m_port_name)
		else:
			raise ValueError(
			    f"Master module '{master.name}' does not have sim port '{m_port_name}'."
			)
		if not slave._hasSlaveSimPort(s_port_name):
			slave._addSlaveSimPort(s_port_name, req_queue_size)
		else:
			if req_queue_size != slave.s_simports[s_port_name]:
				raise ValueError(
				    f"Slave module '{slave.name}' does not have sim port '{s_port_name}'."
				)
		# Record the connection as a tuple
		connection = {
		    "master": master.name,
		    "slave": slave.name,
		    "master_port": m_port_name,
		    "slave_port": s_port_name,
		}
		self.linked_mod_simport_list.append(connection)

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

		[mod.codegen() for mod in self.modules.values()]

		self._create_hh()
		self._create_cc()

	def _create_hh(self):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TCCSimBase.hh")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getIncPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_define_macro(content, self.getIncPath(fullPath=True)),
		        lambda content: replace_namespace_name(content, self.group == "", self.group),
		        lambda content: replace_class_name(content, self.name),
		    )
		)

	def _create_cc(self):
		folder_path = os.path.dirname(os.path.abspath(__file__))
		tpl_path = os.path.join(folder_path, "template", "cc", "TCCSimBase.cc")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getLibPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_class_name(content, self.name),
		        lambda content: replace_inc_self_path(content, self.getIncPath(fullPath=False)),
		        lambda content: replace_namespace_name(content, self.group == "", self.group),
		        lambda content: self.replace_simport(content),
		        lambda content: self.replace_register_modules(content),
		        lambda content: replace_simport_connection(
		            content=content, linked_list=self.linked_mod_simport_list, post_fix="mod"
		        )
		    )
		)

	def replace_register_modules(self, content):
		# Generate include directives for all modules
		includes = "\n".join(
		    f'#include "{mod.getIncPath(fullPath=False)}"' for mod in self.modules.values()
		)
		content = content.replace("{{SimBase.SimBase.register_modules.include}}", includes)

		# Generate and Register Modules
		generate = "\n\t".join(
		    f"auto {name}_mod = new {mod.getName(namespace=True)}(\"{name}\");"
		    for name, mod in self.modules.items()
		)

		generate += "\n\n\t" + "\n\t".join(
		    f"this->addModule({name}_mod);" for name in self.modules.keys()
		)

		content = content.replace("{{SimBase.SimBase.register_modules.generate}}", generate)
		return content
