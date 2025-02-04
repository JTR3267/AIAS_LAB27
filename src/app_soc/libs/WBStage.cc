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

#include "WBStage.hh"

WBStage::WBStage(const std::string& name) : acalsim::SimModule(name) {}

WBStage::~WBStage() {}

void WBStage::init() { CLASS_INFO << "WBStage Initialization"; }

void WBStage::step() {}

void WBStage::execDataPath() {
	// Check for data hazard
	if (this->checkDataHazard(info->inst.a2.reg, info->inst.a3.reg)) {
		dynamic_cast<SimModule*>(this->getSimulator()->getModule("IFStage"))->setStall();
		dynamic_cast<SimModule*>(this->getSimulator()->getModule("IDStage"))->setStall();
	}
	// Write back to the register file
	auto info      = this->mem_wb_reg->get();
	auto inst_type = inst.inst_type;
	switch (inst_type) {
		case instr_type::LW: this->getSimulator()->writeRegister(info->inst.rd, info->mem_val.load_data); break;
		case instr_type::JAL: this->getSimulator()->writeRegister(info->inst.rd, info->mem_val.pc_plus_4_to_rd); break;
		case instr_type::ADD:
		case instr_type::ADDI:
		case instr_type::LUI: this->getSimulator()->writeRegister(info->inst.rd, info->mem_val.alu_out); break;
		case instr_type::SB:
		case instr_type::BEQ: break;
		default: CLASS_ERROR << "Invalid instruction type"; break;
	}
}

bool WBStage::checkDataHazard(int _rs1, int _rs2) {
	// Get rs1 and rs2 from the ID stage inbound register
	auto id_reg = this->getSimulator()->if_->getRegFromID();
	auto rd     = id_reg->get()->inst.a1.reg;
	return (rd == _rs1 || rd == _rs2);
}

bool WBStage::checkHcf() { return (this->mem_wb_reg->get()->inst.inst_type == instr_type::HCF); }
