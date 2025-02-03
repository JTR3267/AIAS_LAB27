#include <memory>

#include "ACALSimSC.hh"
#include "{{util.inc_self_path}}"

int main(int argc, char** argv) {
	// Step 3. instantiate a top-level simulation instance
	acalsim::top = std::make_shared<{{util.class_name}}>();
	acalsim::top->init(argc, argv);
	acalsim::top->run();
	acalsim::top->finish();
	return 0;
}
