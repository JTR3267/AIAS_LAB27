/*
 * Copyright 2023-2024 Playlab/ACAL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_APP_SOC_INCLUDE_SOCSIMTOP_HH_
#define SRC_APP_SOC_INCLUDE_SOCSIMTOP_HH_

#include <string>

#include "ACALSim.hh"
#include "InstMemory.hh"

/**
 * @brief A class representing a simulator template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a simulator.
 */
class SOCSimTop : public acalsim::SimTop {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the simulator.
	 */
	SOCSimTop(const std::string& _configFilePath = "", const std::string& _tracingFileName = "trace");

	SOCSimTop(const std::vector<std::string>& _configFilePaths, const std::string& _tracingFileName = "trace");

	~SOCSimTop();

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

	void registerConfigs() override;
};

#endif  // SRC_APP_SOC_INCLUDE_SOCSIMTOP_HH_
