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

#ifndef SRC_APP_SOC_INCLUDE_IDSTAGE_HH_
#define SRC_APP_SOC_INCLUDE_IDSTAGE_HH_

#include <string>

#include "ACALSim.hh"
#include "CPU.hh"
#include "CPUDefs.hh"
#include "Register.hh"

/**
 * @brief A class representing a component template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a component.
 */
class IDStage : public acalsim::SimModule {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the component.
	 */
	IDStage(const std::string& name, Register<if_stage_out>* _if_id_reg, Register<id_stage_out>* _id_exe_reg);

	~IDStage();

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

	void execDataPath();

	bool getStallStatus() { return this->stall; }

	std::shared_ptr<if_stage_out> getRegInfoFromID() { return this->if_id_reg->get(); }

	void setStall() {
		this->id_exe_reg->setStall();
		this->stall = true;
	}

	void setFlush() {
		this->id_exe_reg->setFlush();
		this->flush = true;
	}

private:
	Register<if_stage_out>* if_id_reg;
	Register<id_stage_out>* id_exe_reg;
	bool                    flush;
	bool                    stall;
};

#endif  // SRC_APP_SOC_INCLUDE_IDSTAGE_HH_
