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

#ifndef SRC_APP_SOC_INCLUDE_CPU_HH_
#define SRC_APP_SOC_INCLUDE_CPU_HH_

/* --------------------------------------------------------------------------------------
 *  A template header file to demonstrate how to create a user-defined libraries.
 *  Feel free to modify, delete, or add any declarations and header file inside `include` folder.
 *  Ensure proper inclusion of header files in your source code.
 * --------------------------------------------------------------------------------------*/

#include <string>

#include "ACALSim.hh"
#include "DataMemory.hh"
#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"
#include "InstMemory.hh"
#include "MEMStage.hh"
#include "WBStage.hh"

/**
 * @brief A class representing a simulator template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a simulator.
 */
class CPU : public acalsim::CPPSimBase {
public:
	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the simulator.
	 */
	CPU(const std::string& _name, const std::string& _m_port, const std::string& _s_port) : acalsim::CPPSimBase(_name) {
		this->imem    = new InstMemory();
		this->m_port_ = this->addMasterPort(_m_port);
		this->s_port_ = this->addSlavePort(_s_port, 1);
	}

	~CPU();

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

	void writeRegister(int _index, uint32_t _data) {}

private:
	InstMemory*          imem;
	acalsim::MasterPort* m_port_;
	acalsim::SlavePort*  s_port_;
	IFStage*             if_;
	IDStage*             id_;
	EXEStage*            exe_;
	MEMStage*            mem_;
	WBStage*             wb_;
};

#endif  // SRC_APP_SOC_INCLUDE_CPU_HH_
