/* --------------------------------------------------------------------------------------
 *  A template to demonstrate how to create your own simulation using ACALSim framework
 *  Step 1. Inherit SimBase to create your own simulator classes
 *
 *  Step 2. Inherit SimTop to create your own top-level simulation class
 * 			1) (optional) SimTop::registerConfigs(); // register SimConfig into ACALSim.
 *			2) (optional) SimTop::registerCLIArguments(); // update Parameter via CLI Option
 *          3) (required) SimTop::registerSimulators(); // Add all the simulators one by one

 *  Step 3. instantiate a top-level simulation instance and call the following APIs in turn
 *          1) SimTop::init(); //Pre-Simulation Initialization
 *          2) SimTop::run();  //Simulation main loop
 *          3) SimTop::finish(); // Post-Simulation cleanup
 * --------------------------------------------------------------------------------------*/

#ifndef {{util.define_macro}}
#define {{util.define_macro}}

/* --------------------------------------------------------------------------------------
 *  A template header file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any declarations and header file inside `include` folder.
 *  Ensure proper inclusion of header files in your source code.
 * --------------------------------------------------------------------------------------*/

#include <string>

#include "ACALSim.hh"

{{util.namespace_name.head}}

/**
 * @brief A class representing a simulator template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a simulator.
 */
class {{util.class_name}} : public acalsim::SimTop {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the simulator.
	 */
	{{util.class_name}}(const std::string& _configFilePath = "", const std::string& _tracingFileName = "trace");

	{{util.class_name}}(const std::vector<std::string>& _configFilePaths, const std::string& _tracingFileName = "trace");

	~{{util.class_name}}();

	/**
	 * @brief Virtual function to control the thread step.
	 *
	 * @details User can override this function to let the control thread do something
	 * 			while all the simulators are processing their own events in each iteration.
	 * 			For example, users can dump statistics periodically here.
	 */
	void control_thread_step() override;

	/**
	 * @brief Create and register all simulators.
	 *
	 * @details This pure virtual function must be implemented by derived classes to
	 * 			create and register all simulators.
	 */
	void registerSimulators() override;

	/**
	 * @brief Performs system initial setup before the simulators are launched.
	 *
	 * This function is responsible for configuring any necessary parameters
	 * and performing setup tasks required before launching the simulators.
	 * It ensures that the environment is prepared and all prerequisites are met.
	 */
	void preSimInitSetup() override;

	/**
	 * @brief Performs system initial setup after the simulators have been launched.
	 *
	 * This function handles tasks that need to be completed once the simulators
	 * are up and running. It may involve post-initialization adjustments,
	 * resource allocation, or other necessary operations to ensure the simulators
	 * function correctly.
	 */
	void postSimInitSetup() override;
};

{{util.namespace_name.footer}}

#endif  // {{util.define_macro}}
