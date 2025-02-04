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

#include "MEMStage.hh"

#include "MemReqPacket.hh"

MEMStage::MEMStage(const std::string& name) : acalsim::SimModule(name) {}

MEMStage::~MEMStage() {}

void MEMStage::init() { CLASS_INFO << "MEMStage Initialization"; }

void MEMStage::step() {}

void MEMStage::processRespPkt(const uint32_t& _data) {
	if (this->status == mem_stage_status::WAIT) {
		// Send the data to the WB stage
		auto mem_out               = std::make_shared<mem_stage_out>();
		mem_out->inst              = inst;
		mem_out->mem_val.load_data = _data;
		this->mem_wb_reg->set(mem_out);
		// Set the status to IDLE
		this->setStatus(mem_stage_status::IDLE);
	} else {
		CLASS_ERROR << "Invalid MEMStage status";
	}
}

void MEMStage::execDataPath() {
	if (this->status == mem_stage_status::IDLE) {
		auto info      = this->exe_mem_reg->get();
		auto inst_type = info->inst.inst_type;
		if (inst_type == instr_type::SB || inst_type == instr_type::BEQ) {
			this->checkMemoryAccess(info);
		} else {
			// Check for data hazard
			if (this->checkDataHazard(info->inst.a2.reg, info->inst.a3.reg)) {
				dynamic_cast<SimModule*>(this->getSimulator()->getModule("IFStage"))->setStall();
				dynamic_cast<SimModule*>(this->getSimulator()->getModule("IDStage"))->setStall();
			}
			this->checkMemoryAccess(info);
		}
	} else {
		CLASS_ERROR << "Invalid MEMStage status";
	}
}

void MEMStage::checkMemoryAccess(const mem_stage_info* _info) {
	auto inst      = _info->inst;
	auto inst_type = inst.inst_type;
	switch (inst_type) {
		case instr_type::SB:
			auto memReq =
			    std::make_shared<MemReqPacket>("MemReq", _info.alu_out, _info.write_data, MemReqPacket::ReqType::WRITE);
			this->sendReqToMemory(memReq);
			break;
		case instr_type::LW:
			auto memReq = std::make_shared<MemReqPacket>("MemReq", _info.alu_out, 0, MemReqPacket::ReqType::READ);
			this->sendReqToMemory(memReq);
			break;
		case instr_type::JAL:
			auto mem_out                     = std::make_shared<mem_stage_out>();
			mem_out->inst                    = inst;
			mem_out->mem_val.pc_plus_4_to_rd = (_info.pc + 4);
			this->mem_wb_reg->set(mem_out);
			break;
		case instr_type::ADD:
		case instr_type::ADDI:
		case instr_type::LUI:
			auto mem_out     = std::make_shared<mem_stage_out>();
			mem_out->inst    = inst;
			mem_out->alu_out = _info.alu_out;
			this->mem_wb_reg->set(mem_out);
			break;
		case instr_type::BEQ:
			auto mem_out  = std::make_shared<mem_stage_out>();
			mem_out->inst = inst;
			this->mem_wb_reg->set(mem_out);
			break;
		default: CLASS_ERROR << "Invalid instruction type"; break;
	}
}

bool MEMStage::checkDataHazard(int _rs1, int _rs2) {
	// Get rs1 and rs2 from the ID stage inbound register
	auto id_reg = this->getSimulator()->if_->getRegFromID();
	auto rd     = id_reg->get()->inst.a1.reg;
	return (rd == _rs1 || rd == _rs2);
}

void MEMStage::sendReqToMemory(const SimPacket* _pkt) {
	if (this->m_port_->isPushReady()) {
		// Set the status to WAIT
		this->setStatus(mem_stage_status::WAIT);
		// Send the packet to Data Memory
		this->m_port_->push(_pkt);
	} else {
		CLASS_ERROR << "[MEMStage] Failed to push the MemReqPacket into MasterPort";
	}
}
