#ifndef {{util.define_macro}}
#define {{util.define_macro}}

/* --------------------------------------------------------------------------------------
 *  A template header file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any declarations and header file inside `include` folder.
 *  Ensure proper inclusion of header files in your source code.
 * --------------------------------------------------------------------------------------*/

#include <string>

#include "ACALSimSC.hh"

{{util.namespace_name.head}}

/**
 * @brief A class representing a simulator template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a simulator.
 */
class {{util.class_name}} : public acalsim::SCSimBase {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the simulator.
	 * @param _cycleDuration The cycle duration for a SystemC-type simulator
	 * @param _totalSimCycles The entire simulation cycles for a SystemC-type simulator
	 */
	{{util.class_name}}(const std::string& name, int _cycleDuration, acalsim::Tick _totalSimCycles);

	~{{util.class_name}}();

	/**
	 * @brief The simulator-level intialization function before the simulation loop starts
	 */
	void init() override;

	/**
	 * @brief The cleanup function called after the simulation loop ends.
	 *
	 * @note Release dynamic memory, clean up the event queue, etc., in this function.
	 */
	void cleanup() override;

	// For a SystemC-type simulator, users can create their SystemC Module (SC_MODULE)
	// before starting the SystemC simulation in this function.
	void registerSystemCSim() override;

	// For a SystemC-type simulator, users can initialize/reset their SystemC Module (SC_MODULE)
	// before starting the SystemC simulation in this function.
	void initSystemCSim() override;

	// For a SystemC-type simulator, users can define actions to be taken
	// before the SystemC simulation starts at the current Tick in this function.
	void preSystemCSim() override;

	// For a SystemC-type simulator, users can define actions to be taken
	// after the SystemC simulation ends at the current Tick in this function.
	void postSystemCSim() override;
};

{{util.namespace_name.footer}}

#endif  // {{util.define_macro}}
