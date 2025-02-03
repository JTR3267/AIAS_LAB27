#include <memory>

#include "ACALSim.hh"

#include "SOCSimTop.hh"

int main(int argc, char** argv) {
	// Step 3. instantiate a top-level simulation instance
	acalsim::top = std::make_shared<SOCSimTop>();
	acalsim::top->init(argc, argv);
	acalsim::top->run();
	acalsim::top->finish();
	return 0;
}
