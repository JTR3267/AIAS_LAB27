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

#include "SOCSimTop.hh"

#include "CPU.hh"
#include "DataMemory.hh"
#include "SOCConfig.hh"

/* --------------------------------------------------------------------------------------
 *  A template source code file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any new content and source code files within the `libs` folder.
 *  Ensure that all project header files referenced are placed inside the `include` folder.
 * --------------------------------------------------------------------------------------*/

SOCSimTop::SOCSimTop(const std::string& _configFilePath, const std::string& _tracingFileName)
    : acalsim::SimTop(_configFilePath, _tracingFileName) {}

SOCSimTop::SOCSimTop(const std::vector<std::string>& _configFilePaths, const std::string& _tracingFileName)
    : acalsim::SimTop(_configFilePaths, _tracingFileName) {}

SOCSimTop::~SOCSimTop() {}

void SOCSimTop::control_thread_step() {}

void SOCSimTop::registerSimulators() {
	int memory_size = acalsim::top->getParameter<int>("SOC", "memory_size");
	// SimPort Name
	std::string CPU_sim_m        = "CPU_sim-m";
	std::string CPU_sim_s        = "CPU_sim-s";
	std::string DataMemory_sim_m = "DataMemory_sim-m";
	std::string DataMemory_sim_s = "DataMemory_sim-s";

	// Generate and Register Simulator
	auto CPU_sim        = new CPU("CPU", CPU_sim_m, CPU_sim_s);
	auto DataMemory_sim = new DataMemory("DataMemory", memory_size, DataMemory_sim_m, DataMemory_sim_s);

	this->addSimulator(CPU_sim);
	this->addSimulator(DataMemory_sim);

	CPU_sim->addDownStream(DataMemory_sim, "DSDMEM");
	DataMemory_sim->addUpStream(CPU_sim, "USCPU");

	// Connect SimPort
	acalsim::SimPortManager::ConnectPort(CPU_sim, DataMemory_sim, CPU_sim_m, DataMemory_sim_s);
	acalsim::SimPortManager::ConnectPort(DataMemory_sim, CPU_sim, DataMemory_sim_m, CPU_sim_s);

	// Connect SimChannel
}

void SOCSimTop::registerConfigs() {
	auto config = new SOCConfig("SOC configuration");
	this->addConfig("SOC", config);
}

void SOCSimTop::preSimInitSetup() {}

void SOCSimTop::postSimInitSetup() {}
