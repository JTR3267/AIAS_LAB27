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

EXEStage::EXEStage(const std::string& name, Register<id_stage_out>* _id_exe_reg, Register<exe_stage_out>* _exe_mem_reg)
    : acalsim::SimModule(name), id_exe_reg(_id_exe_reg), exe_mem_reg(_exe_mem_reg), stall(false) {}

EXEStage::~EXEStage() {}

void EXEStage::init() { CLASS_INFO << "EXEStage Initialization"; }

void EXEStage::step() {}

void EXEStage::execDataPath() {
	if (!this->stall) {
		auto                      id_stage_out = this->id_exe_reg->get();
		uint32_t                  alu_out_, write_data_;
		std::pair<bool, uint32_t> branch_compare;
		switch (id_stage_out->inst.op) {
			case ADD:
				alu_out_    = id_stage_out->rs1_data + id_stage_out->rs2_data;
				write_data_ = 0;
				break;
			case ADDI:
				alu_out_    = id_stage_out->rs2_data + id_stage_out->immediate;
				write_data_ = 0;
				break;
			case LUI:
				alu_out_    = id_stage_out->immediate << 12;
				write_data_ = 0;
				break;
			case BEQ:
				branch_compare.first  = id_stage_out->rs1_data == id_stage_out->rs2_data;
				branch_compare.second = id_stage_out->immediate;
				write_data_           = 0;
				alu_out_              = 0;
				break;
			case JAL:
				branch_compare.first  = true;
				branch_compare.second = id_stage_out->immediate;
				write_data_           = 0;
				alu_out_              = 0;
				break;
			case SB:
				alu_out_    = id_stage_out->rs2_data + id_stage_out->immediate;
				write_data_ = id_stage_out->rs1_data;
				break;
			case LW:
				alu_out_    = id_stage_out->rs2_data + id_stage_out->immediate;
				write_data_ = 0;
				break;
			default:
				alu_out_    = 0;
				write_data_ = 0;
				break;
		}
		std::shared_ptr<exe_stage_out> infoPtr = std::make_shared<exe_stage_out>(exe_stage_out{
		    .pc = id_stage_out->pc, .inst = id_stage_out->inst, .alu_out = alu_out_, .write_data = write_data_});
		this->exe_mem_reg->set(infoPtr);
	}
	this->stall = false;
}
