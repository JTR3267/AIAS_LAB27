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

#include "CPUDefs.hh"
#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"
#include "MEMStage.hh"
#include "Register.hh"
#include "WBStage.hh"

CPU::CPU(const std::string& name) : acalsim::CPPSimBase(name) {
	// Generate and Register MasterPorts

	// Generate and Register SlavePorts

	this->imem = new InstMemory();
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
}

void CPU::registerModules() {
	// Create Register between stages
	auto exe_mem_reg = new<Register<exe_stage_out>>();
	auto mem_wb_reg  = new<Register<mem_stage_out>>();

	// Generate and Register Modules
	this->if_  = new IFStage("IFStage");
	this->id_  = new IDStage("IDStage");
	this->exe_ = new EXEStage("EXEStage");
	this->mem_ = new MEMStage("MEMStage", exe_mem_reg, mem_wb_reg);
	this->wb_  = new WBStage("WBStage", mem_wb_reg);

	this->addModule(if_);
	this->addModule(id_);
	this->addModule(exe_);
	this->addModule(mem_);
	this->addModule(wb_);

	// Connect SimPort
}

void CPU::step() {
	if (this->s_port_->isPopValid()) {
		auto packet = this->s_port_->pop();
		this->accept(acalsim::top->getGlobalTick(), *packet);
	}
}

void CPU::cleanup() {}
