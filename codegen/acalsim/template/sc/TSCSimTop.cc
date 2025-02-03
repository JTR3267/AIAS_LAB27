#include "{{util.inc_self_path}}"

{{SimTop.SimTop.register_simulators.include}}

/* --------------------------------------------------------------------------------------
 *  A template source code file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any new content and source code files within the `libs` folder.
 *  Ensure that all project header files referenced are placed inside the `include` folder.
 * --------------------------------------------------------------------------------------*/

{{util.namespace_name.head}}

{{util.class_name}}::{{util.class_name}}(const std::string& _configFilePath, const std::string& _tracingFileName)
    : acalsim::SCSimTop(_configFilePath, _tracingFileName) {}

{{util.class_name}}::~{{util.class_name}}() {}

void {{util.class_name}}::control_thread_step() {}

void {{util.class_name}}::registerSimulators() {
	// Generate and Register Simulator
	{{SimTop.SimTop.register_simulators.generate}}

	// Connect SimPort
	{{SimPort.simport_connection}}

	// Connect SimChannel
	{{SimChannel.channel_connection}}
}

void {{util.class_name}}::preSimInitSetup() {}

void {{util.class_name}}::postSimInitSetup() {}

{{util.namespace_name.footer}}
