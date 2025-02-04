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

#include "CPU.hh"

#include "CPUSingleIterationEvent.hh"
#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"
#include "MEMStage.hh"
#include "Register.hh"
#include "WBStage.hh"

CPU::CPU(const std::string& name) : acalsim::CPPSimBase(name) {
	this->imem = new InstMemory();
	for (int i = 0; i < 32; i++) { this->regs[i] = new Register<uint32_t>(std::make_shared<uint32_t>(0)); }

	this->if_id_reg   = new Register<if_stage_out>();
	this->id_exe_reg  = new Register<id_stage_out>();
	this->exe_mem_reg = new Register<exe_stage_out>();
	this->mem_wb_reg  = new Register<mem_stage_out>();

	// Generate and Register MasterPorts

	// Generate and Register SlavePorts
}

CPU::~CPU() {}

void CPU::init() {
	CLASS_INFO << "CPU initialization and register SimModules";
	this->registerModules();
	for (auto& [_, module] : this->modules) { module->init(); }

	std::string asm_file_path = acalsim::top->getParameter<std::string>("SOC", "asm_file_path");

	DataMemory* dmem = dynamic_cast<DataMemory*>(this->getDownStream("DSDMEM"));
	this->imem->parse(asm_file_path, static_cast<uint8_t*>(dmem->getMemPtr()));
	this->imem->normalize_labels();

	CLASS_INFO << "Simulation starts";

	auto rc    = acalsim::top->getRecycleContainer();
	auto event = rc->acquire<CPUSingleIterationEvent>(&CPUSingleIterationEvent::renew, this);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
}

void CPU::registerModules() {
	// Generate and Register Modules
	auto IFStage_mod  = new IFStage("IFStage", this->if_id_reg);
	auto IDStage_mod  = new IDStage("IDStage", this->if_id_reg, this->id_exe_reg);
	auto EXEStage_mod = new EXEStage("EXEStage", this->id_exe_reg, this->exe_mem_reg);
	auto MEMStage_mod = new MEMStage("MEMStage");
	auto WBStage_mod  = new WBStage("WBStage");

	this->addModule(IFStage_mod);
	this->addModule(IDStage_mod);
	this->addModule(EXEStage_mod);
	this->addModule(MEMStage_mod);
	this->addModule(WBStage_mod);

	// Connect SimPort
}

void CPU::execDataPath() {
	dynamic_cast<IDStage*>(this->getModule("IDStage"))->execDataPath();
	dynamic_cast<IFStage*>(this->getModule("IFStage"))->execDataPath();
}
void CPU::updatePipeRegisters() {
	this->if_id_reg->update();
	this->id_exe_reg->update();
	this->exe_mem_reg->update();
	this->mem_wb_reg->update();
}
void CPU::updateRegisterFile() {
	for (int i = 0; i < 32; i++) { this->regs[i]->update(); }
}

void CPU::printRegfile() {
	std::ostringstream oss;

	oss << "CPU Simulation is done\n";

	for (int i = 0; i < 32; i++) {
		oss << "x" << std::setw(2) << std::setfill('0') << std::dec << i << ":0x";

		auto val = regs[i]->get();

		uint32_t reg_value = (val) ? *val : 0;

		oss << std::setw(8) << std::setfill('0') << std::hex << reg_value << " ";

		if ((i + 1) % 8 == 0) { oss << "\n"; }
	}

	oss << "\n";

	CLASS_INFO << oss.str();
}

void CPU::updatePC() { dynamic_cast<IFStage*>(this->getModule("IFStage"))->updatePC(); }

void CPU::checkNextCycleEvent() {
	bool stall = dynamic_cast<IFStage*>(this->getModule("IFStage"))->getStallStatus() |
	             dynamic_cast<IDStage*>(this->getModule("IDStage"))->getStallStatus() |
	             dynamic_cast<EXEStage*>(this->getModule("EXEStage"))->getStallStatus();
	// bool hcf = dynamic_cast<EXEStage*>(this->getModule("WBStage"))->checkHcf();
	if (!stall) {
		auto rc    = acalsim::top->getRecycleContainer();
		auto event = rc->acquire<CPUSingleIterationEvent>(&CPUSingleIterationEvent::renew, this);
		this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
	}
}

void CPU::updateSystemStates() {
	this->updatePipeRegisters();
	this->updateRegisterFile();
}

void CPU::step() {}

void CPU::cleanup() {}
