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
			CLASS_INFO << "Process instruction at PC = " << if_stage_out->pc << " : " << if_stage_out->inst.op;
			switch (if_stage_out->inst.op) {
				case instr_type::ADD:
				case instr_type::SUB:
				case instr_type::SLT:
				case instr_type::SLTU:
				case instr_type::AND:
				case instr_type::OR:
				case instr_type::XOR:
				case instr_type::SLL:
				case instr_type::SRL:
				case instr_type::SRA:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a3.reg);
					immediate = 0;
					break;

				case instr_type::ADDI:
				case instr_type::SLTI:
				case instr_type::SLTIU:
				case instr_type::ANDI:
				case instr_type::ORI:
				case instr_type::XORI:
				case instr_type::SLLI:
				case instr_type::SRLI:
				case instr_type::SRAI:
					rs1_data  = 0;
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;

				case instr_type::BEQ:
				case instr_type::BGE:
				case instr_type::BGEU:
				case instr_type::BLT:
				case instr_type::BLTU:
				case instr_type::BNE:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a1.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;

				case instr_type::SB:
					rs1_data  = cpu->readRegister(if_stage_out->inst.a1.reg);
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;
				case instr_type::LW:
					rs1_data  = 0;
					rs2_data  = cpu->readRegister(if_stage_out->inst.a2.reg);
					immediate = if_stage_out->inst.a3.imm;
					break;
				case instr_type::JALR:
					rs1_data  = 0;
					rs2_data  = if_stage_out->inst.a2.reg;
					immediate = if_stage_out->inst.a3.imm;
					break;
				case instr_type::LUI:
					rs1_data  = 0;
					rs2_data  = 0;
					immediate = if_stage_out->inst.a2.imm;
					break;
				case instr_type::HCF:
					CLASS_INFO << "Encountered HCF!";
					rs1_data  = 0;
					rs2_data  = 0;
					immediate = 0;
					break;
				case instr_type::UNIMPL:
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
	this->stall = false;
	this->flush = false;
}
