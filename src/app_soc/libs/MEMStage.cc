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
	if (this->status == mem_stage_status::WAIT || this->stall_ma) {
		if (this->resp_pkt) {
			// Get stall cycle
			if (this->stall_cycle_begin) {
				int  stall_cycle = (int)(acalsim::top->getGlobalTick() - this->stall_cycle_begin) + 1;
				auto req_type    = this->resp_pkt->getType();
				auto cpu         = dynamic_cast<CPU*>(this->getSimulator());
				switch (req_type) {
					case Request::ReqType::WRITE: {
						cpu->getPerfCounter("MemWriteStallCycleCount")->counterPlusN(stall_cycle);
						break;
					}
					case Request::ReqType::READ: {
						cpu->getPerfCounter("MemReadStallCycleCount")->counterPlusN(stall_cycle);
						break;
					}
					default: CLASS_ERROR << "Invalid request type"; break;
				}
				// Reset stall cycle
				stall_cycle_begin = 0;
			} else {
				CLASS_ERROR << "Stall cycle begin is not set in MEM stage";
			}
			// Send the data to the WB stage
			auto info     = std::make_shared<mem_stage_out>();
			info->pc      = this->exe_mem_reg->get()->pc;
			info->inst    = this->exe_mem_reg->get()->inst;
			info->mem_val = {.load_data = this->resp_pkt->getData()};
			this->mem_wb_reg->set(info);

			auto               cpu     = dynamic_cast<CPU*>(this->getSimulator());
			std::string        instStr = cpu->instrToString(info->inst.op);
			std::ostringstream oss;
			oss << "[PC_MEM ] " << std::setw(10) << std::dec << info->pc << " [Inst] " << instStr;
			INFO << oss.str();
			// Set the status to IDLE
			this->setStatus(mem_stage_status::IDLE);
			// Check for data hazard
			if (cpu->checkDataHazard(cpu->getDestReg(info->inst), "MEMStage")) {
				dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStallDH();
				dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStallDH();
			}
		} else {
			this->mem_wb_reg->set(nullptr);
		}
	} else if (this->status == mem_stage_status::IDLE) {
		auto info = this->exe_mem_reg->get();
		if (info) {
			auto               cpu     = dynamic_cast<CPU*>(this->getSimulator());
			std::string        instStr = cpu->instrToString(info->inst.op);
			std::ostringstream oss;
			oss << "[PC_MEM ] " << std::setw(10) << std::dec << info->pc << " [Inst] " << instStr;
			INFO << oss.str();
			auto inst_type = info->inst.op;
			if (inst_type == instr_type::SB || inst_type == instr_type::BEQ) {
				this->checkMemoryAccess(info);
			} else {
				// Check for data hazard
				if (cpu->checkDataHazard(cpu->getDestReg(info->inst), "MEMStage")) {
					dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStallDH();
					dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStallDH();
				}
				this->checkMemoryAccess(info);
			}
		} else {
			this->mem_wb_reg->set(nullptr);
			std::ostringstream oss;
			oss << "[PC_MEM ] " << std::setw(10) << std::dec << ""
			    << " [Inst] NOP";
			INFO << oss.str();
		}
	} else {
		CLASS_ERROR << "Invalid MEMStage status";
	}
}

void MEMStage::checkMemoryAccess(std::shared_ptr<exe_stage_out> _info) {
	auto inst      = _info->inst;
	auto inst_type = inst.op;
	auto cpu       = dynamic_cast<CPU*>(this->getSimulator());
	switch (inst_type) {
		case instr_type::SB: {
			auto data   = new uint32_t(_info->write_data);
			auto memReq = new MemReqPacket("MemReq", Request{.addr = _info->alu_out,
			                                                 .data = data,
			                                                 .type = Request::ReqType::WRITE,
			                                                 .size = Request::ReqSize::BYTE});
			cpu->getPerfCounter("MemWriteRequestCount")->counterPlusOne();
			cpu->getPerfCounter("MemWriteBandwidthRequirement")->counterPlusN(4);
			this->sendReqToMemory(memReq);
			break;
		}
		case instr_type::LW: {
			auto memReq = new MemReqPacket(
			    "MemReq",
			    Request{
			        .addr = _info->alu_out, .data = 0, .type = Request::ReqType::READ, .size = Request::ReqSize::WORD});
			cpu->getPerfCounter("MemReadRequestCount")->counterPlusOne();
			cpu->getPerfCounter("MemReadBandwidthRequirement")->counterPlusN(4);
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
	// Set IF, ID, EXE, MEM stage to stall
	this->setStallMA();
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
		this->stall_cycle_begin = acalsim::top->getGlobalTick();
	} else {
		CLASS_ERROR << "[MEMStage] Failed to push the MemReqPacket into MasterPort";
	}
}
