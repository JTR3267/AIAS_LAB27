#include "CPU.hh"

#include "IFStage.hh"
#include "IDStage.hh"
#include "EXEStage.hh"
#include "MEMStage.hh"
#include "WBStage.hh"


CPU::CPU(const std::string& name) : acalsim::CPPSimBase(name) {

	// Generate and Register MasterPorts
	

	// Generate and Register SlavePorts
	
}

CPU::~CPU() {}

void CPU::init() { 
	CLASS_INFO << "CPU initialization and register SimModules";
	this->registerModules();
	for (auto& [_, module] : this->modules) { module->init(); }
}

void CPU::registerModules() {
	// Generate and Register Modules
	auto IFStage_mod = new IFStage("IFStage");
	auto IDStage_mod = new IDStage("IDStage");
	auto EXEStage_mod = new EXEStage("EXEStage");
	auto MEMStage_mod = new MEMStage("MEMStage");
	auto WBStage_mod = new WBStage("WBStage");

	this->addModule(IFStage_mod);
	this->addModule(IDStage_mod);
	this->addModule(EXEStage_mod);
	this->addModule(MEMStage_mod);
	this->addModule(WBStage_mod);

	// Connect SimPort
	
}

void CPU::step() {}

void CPU::cleanup() {}

