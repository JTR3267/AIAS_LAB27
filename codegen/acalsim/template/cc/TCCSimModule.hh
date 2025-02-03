#ifndef {{util.define_macro}}
#define {{util.define_macro}}


#include <string>

#include "ACALSim.hh"

{{util.namespace_name.head}}

/**
 * @brief A class representing a component template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a component.
 */
class {{util.class_name}} : public acalsim::SimModule {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the component.
	 */
	{{util.class_name}}(const std::string& name);

	~{{util.class_name}}();

	/**
	 * @brief The component-level intialization function before the simulation loop starts
	 */
	void init() override;

	/**
	 * @brief The step function of the component.
	 *
	 * @note This function is executed in every simulation iteration in the simulation loop.
	 * @note Design what the component can do or print out some information here each iteration.
	 */
	void step() override;

};

{{util.namespace_name.footer}}

#endif  // {{util.define_macro}}
