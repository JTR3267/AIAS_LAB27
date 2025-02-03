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


class SimChannelManager:

	def __init__(self):
		self.m_channel_ports = []
		self.s_channel_ports = []

	def _addMasterSimChannel(self, port_name):
		if port_name in self.m_channel_ports:
			raise ValueError(f"MasterChannelPort with name '{port_name}' already exists.")
		self.m_channel_ports.append(port_name)

	def _addSlaveSimChannel(self, port_name):
		if port_name in self.s_channel_ports:
			raise ValueError(f"SlaveChannelPort with name '{port_name}' already exists.")
		self.s_channel_ports.append(port_name)

	def _hasMasterChannelPort(self, name) -> bool:
		return name in self.m_channel_ports

	def _hasSlaveChannelPort(self, name) -> bool:
		return name in self.s_channel_ports


def replace_channel_connection(content, linked_list):
	# Generate simport connection
	simport = "\n\t".join(
	    f'acalsim::ChannelPortManager::ConnectPort({info["master"]}_sim, {info["slave"]}_sim, '
	    f'\"{info["master_port"]}\", \"{info["slave_port"]}\");' for info in linked_list
	)
	return content.replace("{{SimChannel.channel_connection}}", simport)
