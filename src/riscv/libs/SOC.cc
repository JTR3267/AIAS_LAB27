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

#include "SOC.hh"

#include "event/ExecOneInstrEvent.hh"

SOC::SOC(std::string _name) : acalsim::CPPSimBase(_name) {}

void SOC::registerModules() {
	// Get the maximal memory footprint size in the Emulator Configuration
	size_t mem_size = acalsim::top->getParameter<int>("Emulator", "memory_size");

	// Data Memory Timing Model
	this->dmem = new DataMemory("Data Memory", mem_size);

	// Instruction Set Architecture Emulator (Functional Model)
	this->isaEmulator = new Emulator("RISCV RV32I Emulator");

	// CPU Timing Model
	this->cpu = new CPU("Single-Cycle CPU Model", this);

	// register modules
	this->addModule(this->cpu);
	this->addModule(this->dmem);

	// connect modules (connected_module, master port name, slave port name)
	this->cpu->addDownStream(this->dmem, "DSDmem");
	this->dmem->addUpStream(this->cpu, "USCPU");
}

void SOC::simInit() {
	CLASS_INFO << name + " SOC::simInit()!";

	// Initialize the ISA Emulator
	// Parse assmebly file and initialize data memory and instruction memory
	std::string asm_file_path = acalsim::top->getParameter<std::string>("Emulator", "asm_file_path");

	this->isaEmulator->parse(asm_file_path, ((uint8_t*)this->dmem->getMemPtr()), this->cpu->getIMemPtr());
	this->isaEmulator->normalize_labels(this->cpu->getIMemPtr());

	// Initialize all child modules
	for (auto& [_, module] : this->modules) { module->init(); }

	// Inject trigger event
	auto               rc    = acalsim::top->getRecycleContainer();
	ExecOneInstrEvent* event = rc->acquire<ExecOneInstrEvent>(&ExecOneInstrEvent::renew, 1 /*id*/, this->cpu);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
}

void SOC::cleanup() {
	this->cpu->printRegfile();
	CLASS_INFO << "SOC::cleanup() ";
}

void SOC::masterPortRetry(const std::string& port_name) {
	if (port_name == "sIF-m") { this->cpu->retrySendInstPacket(this->getMasterPort("sIF-m")); }
}
