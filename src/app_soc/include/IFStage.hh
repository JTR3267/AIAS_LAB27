#ifndef SRC_APP_SOC_INCLUDE_IFSTAGE_HH_
#define SRC_APP_SOC_INCLUDE_IFSTAGE_HH_

#include <string>

#include "ACALSim.hh"

/**
 * @brief A class representing a component template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a component.
 */
class IFStage : public acalsim::SimModule {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the component.
	 */
	IFStage(const std::string& name);

	~IFStage();

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

#endif  // SRC_APP_SOC_INCLUDE_IFSTAGE_HH_
