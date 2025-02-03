#include "{{util.inc_self_path}}"

{{util.namespace_name.head}}

{{util.class_name}}::{{util.class_name}}(const std::string& name) : acalsim::SimModule(name) {

	{{SimPort.SimPortManager.masterports}}

	{{SimPort.SimPortManager.slaveports}}

}

{{util.class_name}}::~{{util.class_name}}() {}

void {{util.class_name}}::init() {}

void {{util.class_name}}::step() {}

{{util.namespace_name.footer}}
