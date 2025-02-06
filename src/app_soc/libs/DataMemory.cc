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

void DataMemory::processMemoryRequest(Request& _req) {
	CLASS_INFO << "Finish Memory Process at Cycle = " << acalsim::top->getGlobalTick();
	// Check Request type
	if (_req.type == Request::ReqType::READ) {
		// Read data from memory
		auto data_ptr = (uint32_t*)this->readData(_req.addr, sizeof(uint32_t), true);
		// Create MemRespPacket
		auto resp_pkt = new MemRespPacket("MemRespPacket", data_ptr[0]);
		// Send MemRespPacket to the MasterPort
		this->m_port_->push(resp_pkt);
	} else if (_req.type == Request::ReqType::WRITE) {
		CLASS_INFO << "Write data 0x" << std::hex << &_req.data << " to memory at address = " << std::dec << _req.addr;
		// Write data to memory
		this->writeData(&_req.data, _req.addr, sizeof(uint8_t));
		// Create MemRespPacket
		auto resp_pkt = new MemRespPacket("MemRespPacket", 0);
		// Send MemRespPacket to the MasterPort
		this->m_port_->push(resp_pkt);
	}
}

void DataMemory::reqPacketHandler(MemReqPacket* _pkt) {
	CLASS_INFO << "Memory Receive Request at Cycle = " << acalsim::top->getGlobalTick();
	auto req = _pkt->getRequest();
	// Check Request type
	if (req.type == Request::ReqType::READ) {
		CLASS_INFO << "Detect Read Request";
		auto read_latency  = acalsim::top->getParameter<acalsim::Tick>("SOC", "memory_read_latency");
		auto process_event = new MemProcessEvent(this, req);
		this->scheduleEvent(process_event, acalsim::top->getGlobalTick() + read_latency);
	} else if (req.type == Request::ReqType::WRITE) {
		CLASS_INFO << "Detect Write Request";
		auto write_latency = acalsim::top->getParameter<acalsim::Tick>("SOC", "memory_write_latency");
		auto process_event = new MemProcessEvent(this, req);
		this->scheduleEvent(process_event, acalsim::top->getGlobalTick() + write_latency);
	} else {
		CLASS_ERROR << "Invalid Request type detected in DataMemory";
	}
};
