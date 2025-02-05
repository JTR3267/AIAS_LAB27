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

#include "IDStage.hh"
#include "IFStage.hh"

WBStage::WBStage(const std::string& name, Register<mem_stage_out>* _mem_wb_reg)
    : acalsim::SimModule(name), mem_wb_reg(_mem_wb_reg){};

WBStage::~WBStage(){};

void WBStage::init() { CLASS_INFO << "WBStage Initialization"; }

void WBStage::step() {}

void WBStage::execDataPath() {
	auto info = this->mem_wb_reg->get();
	if (info) {
		CLASS_INFO << "Process instruction : " << info->inst.op;
		// Check for data hazard
		if (this->checkDataHazard(info->inst.a2.reg, info->inst.a3.reg)) {
			dynamic_cast<IFStage*>(this->getSimulator()->getModule("IFStage"))->setStall();
			dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->setStall();
		}
		// Write back to the register file
		auto inst_type = info->inst.op;
		auto cpu       = dynamic_cast<CPU*>(this->getSimulator());
		switch (inst_type) {
			case instr_type::LW: {
				cpu->writeRegister(info->inst.a1.reg, info->mem_val.load_data);
				break;
			}
			case instr_type::JAL: {
				cpu->writeRegister(info->inst.a1.reg, info->mem_val.pc_plus_4_to_rd);
				break;
			}
			case instr_type::ADD:
			case instr_type::ADDI:
			case instr_type::AUIPC:
			case instr_type::LUI: {
				cpu->writeRegister(info->inst.a1.reg, info->mem_val.alu_out);
				break;
			}
			case instr_type::SB:
			case instr_type::BEQ:
			case instr_type::HCF: break;
			default: CLASS_ERROR << "Invalid instruction type"; break;
		}
	}
}

bool WBStage::checkDataHazard(int _rs1, int _rs2) {
	// Get rs1 and rs2 from the ID stage inbound register
	auto id_reg = dynamic_cast<IDStage*>(this->getSimulator()->getModule("IDStage"))->getRegInfoFromID();
	if (id_reg) {
		auto rd = id_reg->inst.a1.reg;
		return (rd == _rs1 || rd == _rs2) && (rd != 0);
	}
	return false;
}

bool WBStage::checkHcf() {
	if (this->mem_wb_reg->get()) return this->mem_wb_reg->get()->inst.op == instr_type::HCF;
	return false;
}
