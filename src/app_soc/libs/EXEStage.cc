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

#include "CPU.hh"
#include "IDStage.hh"
#include "IFStage.hh"

EXEStage::EXEStage(const std::string& name, Register<id_stage_out>* _id_exe_reg, Register<exe_stage_out>* _exe_mem_reg)
    : acalsim::SimModule(name), id_exe_reg(_id_exe_reg), exe_mem_reg(_exe_mem_reg), stall_ma(false) {}

EXEStage::~EXEStage() {}

void EXEStage::init() { CLASS_INFO << "EXEStage Initialization"; }

void EXEStage::step() {}

void EXEStage::execDataPath() {
	if (!this->stall_ma) {
		auto info = this->id_exe_reg->get();
		if (info) {
			// Check for data hazard
			auto               cpu     = dynamic_cast<CPU*>(this->getSimulator());
			std::string        instStr = cpu->instrToString(info->inst.op);
			std::ostringstream oss;
			oss << "[PC_EXE ] " << std::setw(10) << std::dec << info->pc << " [Inst] " << instStr;
			INFO << oss.str();
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
					branch_compare.first  = (info->rs1_data == info->rs2_data);
					branch_compare.second = info->immediate;
					write_data_           = 0;
					alu_out_              = 0;
					cpu->getPerfCounter("ConditionalBranchCount")->counterPlusOne();
					if (branch_compare.first) cpu->getPerfCounter("ConditionalBranchHitCount")->counterPlusOne();
					break;
				case JAL:
					branch_compare.first  = true;
					branch_compare.second = info->immediate;
					write_data_           = 0;
					alu_out_              = 0;
					cpu->getPerfCounter("UnconditionalBranchCount")->counterPlusOne();
					cpu->getPerfCounter("UnconditionalBranchHitCount")->counterPlusOne();
					break;
				case SB:
					alu_out_    = info->rs2_data + info->immediate;
					write_data_ = info->rs1_data & 0x000000FF;
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
				auto if_stage = dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"));
				if_stage->setExeNextPC(std::make_pair(true, branch_compare.second));
				if_stage->setFlush();
				dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setFlush();
				cpu->getPerfCounter("FlushCount")->counterPlusOne();
			} else {
				if (cpu->checkDataHazard(cpu->getDestReg(info->inst), "EXEStage")) {
					dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStallDH();
					dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStallDH();
				}
			}
			std::shared_ptr<exe_stage_out> infoPtr = std::make_shared<exe_stage_out>(
			    exe_stage_out{.pc = info->pc, .inst = info->inst, .alu_out = alu_out_, .write_data = write_data_});
			this->exe_mem_reg->set(infoPtr);
		} else {
			this->exe_mem_reg->set(nullptr);
			std::ostringstream oss;
			oss << "[PC_EXE ] " << std::setw(10) << std::dec << ""
			    << " [Inst] NOP";
			INFO << oss.str();
		}
	} else {
		this->exe_mem_reg->set(nullptr);
	}
}
