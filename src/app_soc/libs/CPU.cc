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

#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"
#include "MEMStage.hh"
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
	// Generate and Register Modules
	auto IFStage_mod  = new IFStage("IFStage");
	auto IDStage_mod  = new IDStage("IDStage");
	auto EXEStage_mod = new EXEStage("EXEStage");
	auto MEMStage_mod = new MEMStage("MEMStage");
	auto WBStage_mod  = new WBStage("WBStage");

	this->addModule(IFStage_mod);
	this->addModule(IDStage_mod);
	this->addModule(EXEStage_mod);
	this->addModule(MEMStage_mod);
	this->addModule(WBStage_mod);

	// Connect SimPort
}

void CPU::step() {}

void CPU::cleanup() {}
