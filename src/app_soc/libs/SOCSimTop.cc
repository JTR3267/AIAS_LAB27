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
    : acalsim::SimTop(_configFilePath, _tracingFileName) {
	this->imem = new InstMemory();
}

SOCSimTop::SOCSimTop(const std::vector<std::string>& _configFilePaths, const std::string& _tracingFileName)
    : acalsim::SimTop(_configFilePaths, _tracingFileName) {}

SOCSimTop::~SOCSimTop() {}

void SOCSimTop::control_thread_step() {}

void SOCSimTop::registerSimulators() {
	// Generate and Register Simulator
	auto CPU_sim        = new CPU("CPU");
	auto DataMemory_sim = new DataMemory("DataMemory");

	this->addSimulator(CPU_sim);
	this->addSimulator(DataMemory_sim);

	// Connect SimPort

	// Connect SimChannel
}

void SOCSimTop::registerConfigs() {
	auto config = new SOCConfig("SOC configuration");
	this->addConfig("SOC", config);
}

void SOCSimTop::preSimInitSetup() {
	std::string asm_file_path = acalsim::top->getParameter<std::string>("SOC", "asm_file_path");
	int         memory_size   = acalsim::top->getParameter<int>("SOC", "memory_size");

	this->dmem = new uint8_t[memory_size];
	this->imem->parse(asm_file_path, this->dmem);
	this->imem->normalize_labels();
}

void SOCSimTop::postSimInitSetup() {}
