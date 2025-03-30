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

#include "event/MemReadRespEvent.hh"
#include "event/MemReqDoneEvent.hh"

void DataMemory::step() {
	for (auto s_port : this->s_ports_) {
		if (s_port.second->isPopValid()) {
			auto packet = s_port.second->pop();
			this->accept(acalsim::top->getGlobalTick(), *packet);
		}
	}
}

void DataMemory::cleanup() {
	// this->dumpMemory(0x2000, 0xefff, "memdump.txt");
}

void DataMemory::memReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt) {
	if (auto pkt = dynamic_cast<MemReadReqPacket*>(_memReqPkt)) {
		this->pending_req_queue.push({pkt, 0, 0});
	} else if (auto pkt = dynamic_cast<MemWriteReqPacket*>(_memReqPkt)) {
		if (this->unmatchWriteData != nullptr) {
			this->pending_req_queue.push(
			    {pkt, this->unmatchWriteData->getValidBytes(), this->unmatchWriteData->getData()});
			acalsim::top->getRecycleContainer()->recycle(this->unmatchWriteData);
			this->unmatchWriteData = nullptr;
		} else {
			this->unmatchWriteReq = pkt;
		}
	} else if (auto pkt = dynamic_cast<MemWriteDataPacket*>(_memReqPkt)) {
		if (this->unmatchWriteReq != nullptr) {
			this->pending_req_queue.push({this->unmatchWriteReq, pkt->getValidBytes(), pkt->getData()});
			acalsim::top->getRecycleContainer()->recycle(pkt);
			this->unmatchWriteReq = nullptr;
		} else {
			this->unmatchWriteData = pkt;
		}
	} else {
		CLASS_ERROR << "Wrong memory request packet type!";
	}
	if (this->is_idle) {
		this->is_idle = false;
		triggerNextReq();
	}
}

void DataMemory::triggerNextReq() {
	if (!this->pending_req_queue.empty()) {
		auto queueFront = this->pending_req_queue.front();
		if (auto pkt = dynamic_cast<MemReadReqPacket*>(queueFront.memReqPkt)) {
			memReadReqHandler(pkt);
		} else if (auto pkt = dynamic_cast<MemWriteReqPacket*>(queueFront.memReqPkt)) {
			memWriteReqHandler(pkt, queueFront.validBytes, queueFront.data);
		}
		this->pending_req_queue.pop();
	} else {
		this->is_idle = true;
	}
}

void DataMemory::memReadReqHandler(MemReadReqPacket* _memReadReqPkt) {
	instr      i       = _memReadReqPkt->getInstr();
	instr_type op      = _memReadReqPkt->getOP();
	uint32_t   addr    = _memReadReqPkt->getAddr();
	operand    a1      = _memReadReqPkt->getA1();
	size_t     srcIdx  = _memReadReqPkt->getSrcIdx();
	size_t     destIdx = _memReadReqPkt->getDstIdx();

	size_t   bytes   = 0;
	uint32_t ret     = 0;
	int      latency = acalsim::top->getParameter<acalsim::Tick>("SOC", "memory_read_latency");

	switch (op) {
		case LB:
		case LBU: bytes = 1; break;
		case LH:
		case LHU: bytes = 2; break;
		case LW: bytes = 4; break;
	}

	void* data = this->readData(addr, bytes, false);

	switch (op) {
		case LB: ret = static_cast<uint32_t>(*(int8_t*)data); break;
		case LBU: ret = *(uint8_t*)data; break;
		case LH: ret = static_cast<uint32_t>(*(int16_t*)data); break;
		case LHU: ret = *(uint16_t*)data; break;
		case LW: ret = *(uint32_t*)data; break;
	}

	auto               rc = acalsim::top->getRecycleContainer();
	MemReadRespPacket* memRespPkt =
	    rc->acquire<MemReadRespPacket>(&MemReadRespPacket::renew, i, op, ret, a1, destIdx, srcIdx);

	MemReadRespEvent* memReadRespEvent = rc->acquire<MemReadRespEvent>(&MemReadRespEvent::renew, this, memRespPkt);
	this->scheduleEvent(memReadRespEvent, acalsim::top->getGlobalTick() + latency);

	MemReqDoneEvent* memReqDoneEvent = rc->acquire<MemReqDoneEvent>(&MemReqDoneEvent::renew, this);
	this->scheduleEvent(memReqDoneEvent, acalsim::top->getGlobalTick() + latency);

	rc->recycle(_memReadReqPkt);
}

void DataMemory::memWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, int validBytes, uint32_t data) {
	instr      i    = _memWriteReqPkt->getInstr();
	instr_type op   = _memWriteReqPkt->getOP();
	uint32_t   addr = _memWriteReqPkt->getAddr();

	size_t bytes   = 0;
	int    latency = acalsim::top->getParameter<acalsim::Tick>("SOC", "memory_write_latency");

	switch (op) {
		case SB: bytes = 1; break;
		case SH: bytes = 2; break;
		case SW: bytes = 4; break;
	}

	if (validBytes != 4 && op == SW) {
		void* orgData = this->readData(addr + validBytes, 0, false);
		std::memcpy(&data + validBytes, orgData, 4 - validBytes);
	}

	switch (op) {
		case SB: {
			uint8_t val8 = static_cast<uint8_t>(data);
			this->writeData(&val8, addr, 1);
			break;
		}
		case SH: {
			uint16_t val16 = static_cast<uint16_t>(data);
			this->writeData(&val16, addr, 2);
			break;
		}
		case SW: {
			uint32_t val32 = static_cast<uint32_t>(data);
			this->writeData(&val32, addr, 4);
			break;
		}
	}

	auto             rc              = acalsim::top->getRecycleContainer();
	MemReqDoneEvent* memReqDoneEvent = rc->acquire<MemReqDoneEvent>(&MemReqDoneEvent::renew, this);
	this->scheduleEvent(memReqDoneEvent, acalsim::top->getGlobalTick() + latency);

	rc->recycle(_memWriteReqPkt);
}

void DataMemory::dumpMemory(uint32_t start, uint32_t end, std::string file_path) {
	std::ofstream file(file_path, std::ios::binary | std::ios::out);

	for (uint32_t i = start; i <= end; i++) {
		void*   data = this->readData(i, 0, false);
		uint8_t ret  = *(uint8_t*)data;
		file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ret) << std::endl;
	}

	file.close();
}

void DataMemory::sendReadResp(MemReadRespPacket* readRespPkt) {
	if (!this->getPipeRegister("mem2bus-resp-m")->isStalled()) {
		if (!this->getPipeRegister("mem2bus-resp-m")->push(readRespPkt)) {
			CLASS_ERROR << "Cant push read resp from mem to bus";
		}
	} else {
		CLASS_ERROR << "Memory read response is stalled";
	}
}
