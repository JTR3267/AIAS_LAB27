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

#include "IDStage.hh"

IDStage::IDStage(const std::string& name, Register<if_stage_out>* _if_id_reg, Register<id_stage_out>* _id_exe_reg)
    : acalsim::SimModule(name), if_id_reg(_if_id_reg), id_exe_reg(_id_exe_reg), flush(false), stall(false) {}

IDStage::~IDStage() {}

void IDStage::init() { CLASS_INFO << "IDStage Initialization"; }

void IDStage::step() {}

void IDStage::execDataPath() {
	auto if_stage_out = this->if_id_reg->get();
	auto cpu          = dynamic_cast<CPU*>(this->getSimulator());

	uint32_t rs1_data, rs2_data, immediate;

	if (!this->flush && !this->stall) {
		if (if_stage_out) {
			switch (if_stage_out->inst.op) {
				case ADD:
				case SUB:
				case SLT:
				case SLTU:
				case AND:
				case OR:
				case XOR:
				case SLL:
				case SRL:
				case SRA:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a3.reg);
					immediate = 0;
					break;

				case ADDI:
				case SLTI:
				case SLTIU:
				case ANDI:
				case ORI:
				case XORI:
				case SLLI:
				case SRLI:
				case SRAI:
					rs1_data  = 0;
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;

				case BEQ:
				case BGE:
				case BGEU:
				case BLT:
				case BLTU:
				case BNE:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a1.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;

				case SB:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a1.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;
				case LW:
					rs1_data  = 0;
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;
				case JALR:
					rs1_data  = 0;
					rs2_data  = if_stage_out->inst.a2.reg;
					immediate = if_stage_out->inst.a3.imm;
					break;
				case LUI:
					rs1_data  = 0;
					rs2_data  = 0;
					immediate = if_stage_out->inst.a2.imm;

				case HCF:
				case UNIMPL:
				default:
					rs1_data  = 0;
					rs2_data  = 0;
					immediate = 0;
					break;
			}
			std::shared_ptr<id_stage_out> infoPtr =
			    std::make_shared<id_stage_out>(id_stage_out{.pc        = if_stage_out->pc,
			                                                .inst      = if_stage_out->inst,
			                                                .rs1_data  = rs1_data,
			                                                .rs2_data  = rs2_data,
			                                                .immediate = immediate});
			this->id_exe_reg->set(infoPtr);
		}
	}
}
