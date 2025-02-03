#include "{{util.inc_self_path}}"

{{SimBase.SimBase.register_modules.include}}

{{util.namespace_name.head}}

{{util.class_name}}::{{util.class_name}}(const std::string& name) : acalsim::CPPSimBase(name) {

	// Generate and Register MasterPorts
	{{SimPort.SimPortManager.masterports}}

	// Generate and Register SlavePorts
	{{SimPort.SimPortManager.slaveports}}
}

{{util.class_name}}::~{{util.class_name}}() {}

void {{util.class_name}}::init() { this->registerModules(); }

void {{util.class_name}}::registerModules() {
	// Generate and Register Modules
	{{SimBase.SimBase.register_modules.generate}}

	// Connect SimPort
	{{SimPort.simport_connection}}
}

void {{util.class_name}}::step() {}

void {{util.class_name}}::cleanup() {}

{{util.namespace_name.footer}}
