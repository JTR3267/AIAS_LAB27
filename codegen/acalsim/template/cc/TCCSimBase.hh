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
class {{util.class_name}} : public acalsim::CPPSimBase {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the simulator.
	 */
	{{util.class_name}}(const std::string& name);

	~{{util.class_name}}();

	/**
	 * @brief The simulator-level intialization function before the simulation loop starts
	 */
	void init() override;

	void registerModules();

	/**
	 * @brief The step function of the simulator.
	 *
	 * @note This function is executed in every simulation iteration in the simulation loop.
	 * @note Design what the simulator can do or print out some information here each iteration.
	 */
	void step() override;

	/**
	 * @brief The cleanup function called after the simulation loop ends.
	 *
	 * @note Release dynamic memory, clean up the event queue, etc., in this function.
	 */
	void cleanup() override;

};

{{util.namespace_name.footer}}

#endif  // {{util.define_macro}}
