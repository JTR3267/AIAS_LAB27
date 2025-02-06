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

#include "EXEStage.hh"

#include "IDStage.hh"
#include "IFStage.hh"

EXEStage::EXEStage(const std::string& name, Register<id_stage_out>* _id_exe_reg, Register<exe_stage_out>* _exe_mem_reg)
    : acalsim::SimModule(name), id_exe_reg(_id_exe_reg), exe_mem_reg(_exe_mem_reg), stall(false) {}

EXEStage::~EXEStage() {}

void EXEStage::init() { CLASS_INFO << "EXEStage Initialization"; }

void EXEStage::step() {}

void EXEStage::execDataPath() {
	if (!this->stall) {
		auto info = this->id_exe_reg->get();
		if (info) {
			// Check for data hazard
			if (this->checkDataHazard(info->inst.a1.reg)) {
				dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStall();
				dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStall();
			}
			CLASS_INFO << "Process instruction at PC = " << info->pc;
			uint32_t                  alu_out_, write_data_;
			std::pair<bool, uint32_t> branch_compare;
			switch (info->inst.op) {
				case ADD:
					alu_out_    = info->rs1_data + info->rs2_data;
					write_data_ = 0;
					break;
				case ADDI:
					alu_out_    = info->rs2_data + info->immediate;
					write_data_ = 0;
					break;
				case LUI:
					alu_out_    = info->immediate << 12;
					write_data_ = 0;
					break;
				case BEQ:
					branch_compare.first  = info->rs1_data == info->rs2_data;
					branch_compare.second = info->immediate;
					write_data_           = 0;
					alu_out_              = 0;
					break;
				case JAL:
					branch_compare.first  = true;
					branch_compare.second = info->immediate;
					write_data_           = 0;
					alu_out_              = 0;
					break;
				case SB:
					alu_out_    = info->rs2_data + info->immediate;
					write_data_ = info->rs1_data & 0xFF;
					break;
				case LW:
					alu_out_    = info->rs2_data + info->immediate;
					write_data_ = 0;
					break;
				default:
					alu_out_    = 0;
					write_data_ = 0;
					break;
			}
			if (branch_compare.first) {
				dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setFlush();
				dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setFlush();
			}
			std::shared_ptr<exe_stage_out> infoPtr = std::make_shared<exe_stage_out>(
			    exe_stage_out{.pc = info->pc, .inst = info->inst, .alu_out = alu_out_, .write_data = write_data_});
			this->exe_mem_reg->set(infoPtr);
		}
	}
}

bool EXEStage::checkDataHazard(int _rd) {
	// Get rs1 and rs2 from the ID stage inbound register
	auto id_reg = dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->getRegInfoFromID();
	int  rs1    = id_reg->inst.a2.reg;
	int  rs2    = id_reg->inst.a3.reg;
	if (id_reg) {
		CLASS_INFO << "Exe detect rd = " << _rd << " rs1 = " << rs1 << " rs2 = " << rs2;
		return (_rd == rs1 || _rd == rs2) && (_rd != 0);
	}
	return false;
}
