#include "DataMemory.hh"

DataMemory::DataMemory(const std::string& name) : acalsim::CPPSimBase(name) {

	// Generate and Register MasterPorts
	

	// Generate and Register SlavePorts
	
}

DataMemory::~DataMemory() {}

void DataMemory::init() { 
	CLASS_INFO << "DataMemory Initialization";
	this->registerModules(); 
}

void DataMemory::registerModules() {
	// Generate and Register Modules
	

	

	// Connect SimPort
	
}

void DataMemory::step() {}

void DataMemory::cleanup() {}

