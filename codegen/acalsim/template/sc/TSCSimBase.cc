#include "{{util.inc_self_path}}"

/* --------------------------------------------------------------------------------------
 *  A template source code file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any new content and source code files within the `libs` folder.
 *  Ensure that all project header files referenced are placed inside the `include` folder.
 * --------------------------------------------------------------------------------------*/

{{util.namespace_name.head}}

{{util.class_name}}::{{util.class_name}}(const std::string& name, int _cycleDuration, acalsim::Tick _totalSimCycles)
    : acalsim::SCSimBase(name, _cycleDuration, _totalSimCycles) {}

{{util.class_name}}::~{{util.class_name}}() {}

void {{util.class_name}}::init() {}

void {{util.class_name}}::cleanup() {}

void {{util.class_name}}::registerSystemCSim() {}

void {{util.class_name}}::initSystemCSim() {}

void {{util.class_name}}::preSystemCSim() {}

void {{util.class_name}}::postSystemCSim() {}

{{util.namespace_name.footer}}
