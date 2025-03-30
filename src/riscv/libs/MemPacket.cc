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

#include "MemPacket.hh"

#include "CPU.hh"
#include "DMA.hh"
#include "DataMemory.hh"
#include "SystolicArray.hh"

void MemReadReqPacket::renew(const instr& _i, instr_type _op, uint32_t _addr, operand _a1, size_t _src_idx,
                             size_t _dst_idx) {
	this->acalsim::crossbar::CrossBarPacket::renew(_src_idx, _dst_idx);
	this->i    = _i;
	this->op   = _op;
	this->addr = _addr;
	this->a1   = _a1;
}

void MemReadReqPacket::visit(acalsim::Tick _when, acalsim::SimModule& _module) {
	CLASS_ERROR << "void MemReadReqPacket::visit (SimModule& module) is not implemented yet!";
}

void MemReadReqPacket::visit(acalsim::Tick _when, acalsim::SimBase& _simulator) {
	if (auto dmem = dynamic_cast<DataMemory*>(&_simulator)) {
		dmem->memReqHandler(_when, this);
	} else if (auto dma = dynamic_cast<DMA*>(&_simulator)) {
		dma->dmaReqHandler(_when, this);
	} else if (auto sa = dynamic_cast<SystolicArray*>(&_simulator)) {
		sa->saReqHandler(_when, this);
	} else {
		CLASS_ERROR << "Invalid simulator type!";
	}
}

void MemWriteReqPacket::renew(const instr& _i, instr_type _op, uint32_t _addr, size_t _src_idx, size_t _dst_idx) {
	this->acalsim::crossbar::CrossBarPacket::renew(_src_idx, _dst_idx);
	this->i    = _i;
	this->op   = _op;
	this->addr = _addr;
}

void MemWriteReqPacket::visit(acalsim::Tick _when, acalsim::SimModule& _module) {
	CLASS_ERROR << "void MemWriteReqPacket::visit (SimModule& module) is not implemented yet!";
}

void MemWriteReqPacket::visit(acalsim::Tick _when, acalsim::SimBase& _simulator) {
	if (auto dmem = dynamic_cast<DataMemory*>(&_simulator)) {
		dmem->memReqHandler(_when, this);
	} else if (auto dma = dynamic_cast<DMA*>(&_simulator)) {
		dma->dmaReqHandler(_when, this);
	} else if (auto sa = dynamic_cast<SystolicArray*>(&_simulator)) {
		sa->saReqHandler(_when, this);
	} else {
		CLASS_ERROR << "Invalid simulator type!";
	}
}

void MemWriteDataPacket::renew(uint32_t _data, int _validBytes, size_t _src_idx, size_t _dst_idx) {
	this->acalsim::crossbar::CrossBarPacket::renew(_src_idx, _dst_idx);
	this->data       = _data;
	this->validBytes = _validBytes;
}

void MemWriteDataPacket::visit(acalsim::Tick _when, acalsim::SimModule& _module) {
	CLASS_ERROR << "void MemWriteDataPacket::visit (SimModule& module) is not implemented yet!";
}

void MemWriteDataPacket::visit(acalsim::Tick _when, acalsim::SimBase& _simulator) {
	if (auto dmem = dynamic_cast<DataMemory*>(&_simulator)) {
		dmem->memReqHandler(_when, this);
	} else if (auto dma = dynamic_cast<DMA*>(&_simulator)) {
		dma->dmaReqHandler(_when, this);
	} else if (auto sa = dynamic_cast<SystolicArray*>(&_simulator)) {
		sa->saReqHandler(_when, this);
	} else {
		CLASS_ERROR << "Invalid simulator type!";
	}
}

void MemReadRespPacket::renew(const instr& _i, instr_type _op, uint32_t _data, operand _a1, size_t _src_idx,
                              size_t _dst_idx) {
	this->acalsim::crossbar::CrossBarPacket::renew(_src_idx, _dst_idx);
	this->i    = _i;
	this->op   = _op;
	this->data = _data;
	this->a1   = _a1;
}

void MemReadRespPacket::visit(acalsim::Tick _when, acalsim::SimModule& _module) {
	CLASS_ERROR << "void MemReadRespPacket::visit (SimModule& module) is not implemented yet!";
}

void MemReadRespPacket::visit(acalsim::Tick _when, acalsim::SimBase& _simulator) {
	if (auto cpu = dynamic_cast<CPU*>(&_simulator)) {
		cpu->cpuReadRespHandler(_when, this);
	} else if (auto dma = dynamic_cast<DMA*>(&_simulator)) {
		dma->dmaReadRespHandler(_when, this);
	} else {
		CLASS_ERROR << "Invalid simulator type!";
	}
}
