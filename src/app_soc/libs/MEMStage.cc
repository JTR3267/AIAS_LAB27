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

#include "CPU.hh"
#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"

MEMStage::MEMStage(const std::string& _name, Register<exe_stage_out>* _exe_mem_reg,
                   Register<mem_stage_out>* _mem_wb_reg)
    : acalsim::SimModule(_name), exe_mem_reg(_exe_mem_reg), mem_wb_reg(_mem_wb_reg){};

MEMStage::~MEMStage() {}

void MEMStage::init() { CLASS_INFO << "MEMStage Initialization"; }

void MEMStage::step() {}

void MEMStage::execDataPath() {
	if (this->status == mem_stage_status::WAIT) {
		if (this->resp_pkt) {
			// Send the data to the WB stage
			auto info  = std::make_shared<mem_stage_out>();
			info->pc   = this->exe_mem_reg->get()->pc;
			info->inst = this->exe_mem_reg->get()->inst;
			CLASS_INFO << "Data = 0x" << std::hex << this->resp_pkt->getData();
			info->mem_val = {.load_data = this->resp_pkt->getData()};
			this->mem_wb_reg->set(info);
			// Set the status to IDLE
			this->setStatus(mem_stage_status::IDLE);
		}
	} else if (this->status == mem_stage_status::IDLE) {
		auto info = this->exe_mem_reg->get();
		if (info) {
			CLASS_INFO << "Process instruction at PC = " << info->pc << ", inst = " << info->inst.op;
			auto inst_type = info->inst.op;
			if (inst_type == instr_type::SB || inst_type == instr_type::BEQ) {
				this->checkMemoryAccess(info);
			} else {
				// Check for data hazard
				auto cpu = dynamic_cast<CPU*>(this->getSimulator());
				if (cpu->checkDataHazard(cpu->getDestReg(info->inst), "MEMStage")) {
					CLASS_INFO << "Data hazard detected in EXEStage";
					dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStallDH();
					dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStallDH();
				}
				this->checkMemoryAccess(info);
			}
		} else {
			this->mem_wb_reg->set(nullptr);
			CLASS_INFO << "NOP";
		}
	} else {
		CLASS_ERROR << "Invalid MEMStage status";
	}
}

void MEMStage::checkMemoryAccess(std::shared_ptr<exe_stage_out> _info) {
	auto inst      = _info->inst;
	auto inst_type = inst.op;
	switch (inst_type) {
		case instr_type::SB: {
			CLASS_INFO << "SB instruction";
			auto data = new uint32_t(_info->write_data);
			CLASS_INFO << "Write data = 0x" << std::hex << *data;
			auto memReq = new MemReqPacket("MemReq", Request{.addr = _info->alu_out,
			                                                 .data = data,
			                                                 .type = Request::ReqType::WRITE,
			                                                 .size = Request::ReqSize::BYTE});
			this->sendReqToMemory(memReq);
			break;
		}
		case instr_type::LW: {
			CLASS_INFO << "LW instruction";
			auto memReq = new MemReqPacket(
			    "MemReq",
			    Request{
			        .addr = _info->alu_out, .data = 0, .type = Request::ReqType::READ, .size = Request::ReqSize::WORD});
			this->sendReqToMemory(memReq);
			break;
		}
		case instr_type::JAL: {
			auto info     = std::make_shared<mem_stage_out>();
			info->pc      = _info->pc;
			info->inst    = inst;
			info->mem_val = {.pc_plus_4_to_rd = (_info->pc + 4)};
			this->mem_wb_reg->set(info);
			break;
		}
		case instr_type::ADD:
		case instr_type::ADDI:
		case instr_type::LUI: {
			auto info     = std::make_shared<mem_stage_out>();
			info->pc      = _info->pc;
			info->inst    = inst;
			info->mem_val = {.alu_out = _info->alu_out};
			this->mem_wb_reg->set(info);
			break;
		}
		case instr_type::HCF:
		case instr_type::UNIMPL:
		case instr_type::BEQ: {
			auto info  = std::make_shared<mem_stage_out>();
			info->pc   = _info->pc;
			info->inst = inst;
			this->mem_wb_reg->set(info);
			break;
		}
		default: CLASS_ERROR << "Invalid instruction type = " << inst_type; break;
	}
}

void MEMStage::sendReqToMemory(MemReqPacket* _pkt) {
	// Set IF, ID, EXE stage to stall
	dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStallMA();
	dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStallMA();
	dynamic_cast<EXEStage*>(this->getSimulator()->getModule("EXEStage"))->setStallMA();
	// Send the packet to the MasterPort
	auto m_port = dynamic_cast<CPU*>(this->getSimulator())->getMasterPort();
	if (m_port->isPushReady()) {
		// Set the status to WAIT
		this->setStatus(mem_stage_status::WAIT);
		// Send the packet to Data Memory
		m_port->push(_pkt);
	} else {
		CLASS_ERROR << "[MEMStage] Failed to push the MemReqPacket into MasterPort";
	}
}
