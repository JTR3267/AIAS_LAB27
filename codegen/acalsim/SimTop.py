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

from .SimBase import SimBase
from .SimPort import SimPortManager, replace_simport_connection
from .SimChannel import replace_channel_connection
from .util import (
    create,
    chain_lambdas,
    replace_define_macro,
    replace_class_name,
    replace_namespace_name,
    replace_inc_self_path,
)


class SimTop(SimPortManager):
	"""
    SimTop: Manages and connects multiple SimBase instances, serving as the top-level simulation controller.
    """

	def __init__(self, name: str, app: str, group: str = ""):
		"""
        Initialize SimTop.

        Args:
            name (str): The name of this SimTop instance.
            app (str): The application or base folder name.
            group (str): The group or category of this SimTop instance.
        """
		super().__init__()
		self.name: str = name
		self.group: str = group
		self.app: str = app
		self.systemc: bool = False
		self.simulators: dict[str, SimBase] = {}  # Holds added SimBase instances

		# Lists to store simulator connections
		self.linked_sim_channel_list: list[dict] = []
		self.linked_sim_simport_list: list[dict] = []

	def addSimulator(self, sim: SimBase):
		"""
		Add a simulator to this SimTop instance.

		Args:
			sim (SimBase): The simulator instance to add.

		Raises:
			ValueError: If a simulator with the same name already exists.
		"""
		if sim.name in self.simulators:
			raise ValueError(f"Simulator with name '{sim.name}' already exists.")

		if sim.systemc:
			self.systemc = True  # Flag if any simulator uses SystemC
			# Raise an error or warning that SystemC is not yet supported
			raise NotImplementedError(
			    "SystemC simulator is not integrated into code generation yet. "
			    "Please complete the integration before adding SystemC simulators."
			)

		# Set Application
		if not sim.shared: sim.app = self.app

		self.simulators[sim.name] = sim

	def connectSimChannel(
	    self, master: SimBase, slave: SimBase, m_port_name: str, s_port_name: str
	):
		"""
		Connect a simulation channel between a master and a slave simulator.

		Args:
			master (SimBase): The master simulator.
			slave (SimBase): The slave simulator.
			m_port_name (str): The name of the master port.
			s_port_name (str): The name of the slave port.

		Raises:
			ValueError: If simulators or ports are invalid or not added to this SimTop instance.
		"""
		if master.name not in self.simulators or slave.name not in self.simulators:
			raise ValueError("Both master and slave simulators must be added to SimTop first.")

		if not master._hasMasterChannelPort(m_port_name):
			master._addMasterSimChannel(m_port_name)
		else:
			raise ValueError(f"Master simulator '{master.name}' already has port '{m_port_name}'.")

		if not slave._hasSlaveChannelPort(s_port_name):
			slave._addSlaveSimChannel(s_port_name)
		else:
			raise ValueError(f"Slave simulator '{slave.name}' already has port '{s_port_name}'.")

		self.linked_sim_channel_list.append({
		    "master": master.name,
		    "slave": slave.name,
		    "master_port": m_port_name,
		    "slave_port": s_port_name,
		})

	def connectSimPort(
	    self,
	    master: SimBase,
	    slave: SimBase,
	    m_port_name: str,
	    s_port_name: str,
	    req_queue_size: int = 1
	):
		"""
		Connect a simulation port between a master and a slave module.

		Args:
			master (SimBase): The master simulator.
			slave (SimBase): The slave module.
			m_port_name (str): The name of the master port.
			s_port_name (str): The name of the slave port.
			req_queue_size (int): The request queue size. Defaults to 1.

		Raises:
			ValueError: If simulators or ports are invalid or not added to this SimTop instance.
		"""
		if master.name not in self.simulators or slave.name not in self.simulators:
			raise ValueError("Both master and slave simulators must be added to SimTop first.")

		if not master._hasMasterSimPort(m_port_name):
			master._addMasterSimPort(m_port_name)
		else:
			raise ValueError(f"Master simulator '{master.name}' already has port '{m_port_name}'.")

		if slave._hasSlaveSimPort(s_port_name):
			if req_queue_size != slave.s_simports[s_port_name]:
				raise ValueError(
				    f"Slave simulator '{slave.name}' has incompatible port size for '{s_port_name}'."
				)
		else:
			slave._addSlaveSimPort(s_port_name, req_queue_size)

		self.linked_sim_simport_list.append({
		    "master": master.name,
		    "slave": slave.name,
		    "master_port": m_port_name,
		    "slave_port": s_port_name,
		})

	def getFolderPath(self, type: str, fullPath: bool = True) -> str:
		return os.path.join("src", self.app, type,
		                    self.group) if fullPath else os.path.join(self.group)

	def getIncPath(self, fullPath: bool = True) -> str:
		return os.path.join(self.getFolderPath("include", fullPath), f"{self.name}.hh")

	def getLibPath(self, fullPath: bool = True) -> str:
		return os.path.join(self.getFolderPath("libs", fullPath), f"{self.name}.cc")

	def getName(self, namespace: bool = False) -> str:
		return f"{self.group}::{self.name}" if self.group and namespace else self.name

	def codegen(self):
		if self.app == "":
			raise ValueError("The 'app' attribute cannot be an empty string.")

		[sim.codegen() for sim in self.simulators.values()]

		self._create_hh()
		self._create_cc()

	def _create_hh(self):
		## Get and Read Template
		folder_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "template")
		tpl_path: str
		if self.systemc:

			tpl_path = os.path.join(folder_path, "sc", f"TSCSimTop.hh")
		else:
			tpl_path = os.path.join(folder_path, "cc", f"TCCSimTop.hh")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getIncPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_define_macro(content, self.getIncPath()),
		        lambda content: replace_class_name(content, self.name),
		        lambda content: replace_namespace_name(content, self.group == "", self.group)
		    )
		)

	def _create_cc(self):
		## Get and Read Template
		folder_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "template")
		tpl_path: str
		if self.systemc:
			tpl_path = os.path.join(folder_path, "sc", f"TSCSimTop.cc")
		else:
			tpl_path = os.path.join(folder_path, "cc", f"TCCSimTop.cc")

		create(
		    tpl_path=tpl_path,
		    dst_path=self.getLibPath(fullPath=True),
		    func=chain_lambdas(
		        lambda content: replace_class_name(content, self.name),
		        lambda content: replace_namespace_name(content, self.group == "", self.group),
		        lambda content: replace_inc_self_path(content, self.getIncPath(fullPath=False)),
		        lambda content: self.replace_register_simulators(content),
		        lambda content:
		        replace_simport_connection(content, self.linked_sim_simport_list, "sim"),
		        lambda content: replace_channel_connection(content, self.linked_sim_channel_list),
		    )
		)

	def replace_register_simulators(self, content: str):
		# Generate include directives for all simulators
		includes = "\n".join(
		    f'#include "{sim.getIncPath(fullPath=False)}"' for sim in self.simulators.values()
		)
		content = content.replace("{{SimTop.SimTop.register_simulators.include}}", includes)

		# Generate and Register Simulator
		generate = "\n\t".join(
		    f"auto {name}_sim = new {mod.getName(namespace=True)}(\"{name}\");"
		    for name, mod in self.simulators.items()
		)

		generate += "\n\n\t" + "\n\t".join(
		    f"this->addSimulator({name}_sim);" for name in self.simulators.keys()
		)

		content = content.replace("{{SimTop.SimTop.register_simulators.generate}}", generate)
		return content
