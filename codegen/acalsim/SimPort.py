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


class SimPortManager:

	def __init__(self):
		self.m_simports: list[str] = []
		self.s_simports: dict[str, int] = {}

	def _addMasterSimPort(self, port_name: str):
		if port_name in self.m_simports:
			raise ValueError(f"SlavePort with name '{port_name}' already exists.")
		self.m_simports.append(port_name)

	def _addSlaveSimPort(self, port_name: str, req_queue_size: int):
		if port_name in self.s_simports:
			raise ValueError(f"SlavePort with name '{port_name}' already exists.")
		self.s_simports[port_name] = req_queue_size

	def _hasMasterSimPort(self, name: str) -> bool:
		return name in self.m_simports

	def _hasSlaveSimPort(self, name: str) -> bool:
		return name in self.s_simports

	def replace_simport(self, content):
		return self.replace_masterports(self.replace_slaveports(content))

	def replace_masterports(self, content):
		ports = "\n\t".join(f'this->addMasterPort(\"{name}\");' for name in self.m_simports)
		return content.replace("{{SimPort.SimPortManager.masterports}}", ports)

	def replace_slaveports(self, content):
		ports = "\n\t".join(
		    f'this->addSlavePort(\"{name}\", {size});' for name, size in self.s_simports.items()
		)
		return content.replace("{{SimPort.SimPortManager.slaveports}}", ports)


def replace_simport_connection(content, linked_list: dict[str], post_fix: str):
	# Generate simport connection
	simport = "\n\t".join(
	    f'acalsim::SimPortManager::ConnectPort({info["master"]}_{post_fix}, {info["slave"]}_{post_fix}, '
	    f'\"{info["master_port"]}\", \"{info["slave_port"]}\");' for info in linked_list
	)
	return content.replace("{{SimPort.simport_connection}}", simport)
