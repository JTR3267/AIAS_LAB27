/*
 * Copyright 2023-2024 Playlab/ACAL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DataMemory.hh"

DataMemory::DataMemory(const std::string& name, size_t _size, const std::string& _m_port, const std::string& _s_port)
    : acalsim::CPPSimBase(name), BaseMemory(_size) {
	// Generate and Register MasterPorts
	this->m_port_ = this->addMasterPort(_m_port);
	// Generate and Register SlavePorts
	this->s_port_ = this->addSlavePort(_s_port, 1);
}

DataMemory::~DataMemory(){};

void DataMemory::init() {
	CLASS_INFO << "DataMemory Initialization";
	this->registerModules();
}

void DataMemory::registerModules() {
	// Generate and Register Modules

	// Connect SimPort
}

void DataMemory::step() {
	if (this->s_port_->isPopValid()) {
		auto packet = this->s_port_->pop();
		this->accept(acalsim::top->getGlobalTick(), *packet);
	}
}

void DataMemory::cleanup() {}

void DataMemory::processReqPkt(){};
