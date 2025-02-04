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
#include "InstMemory.hh"
#include "Register.hh"

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
	CPU(const std::string& name);

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

	void execDataPath();
	void updateSystemStates();
	void updatePipeRegisters();
	void updateRegisterFile();
	void updatePC();
	void checkNextCycleEvent();

	void printRegfile();

	const instr& fetchInstr(int index) { return this->imem->fetchInstr(index); }

	const uint32_t& readRegister(int index) {
		CLASS_ASSERT(index >= 0 && index < 32);
		return *(this->regs[index]->get());
	}

private:
	InstMemory*              imem;
	Register<uint32_t>*      regs[32];
	Register<if_stage_out>*  if_id_reg;
	Register<id_stage_out>*  id_exe_reg;
	Register<exe_stage_out>* exe_mem_reg;
	Register<mem_stage_out>* mem_wb_reg;
};

#endif  // SRC_APP_SOC_INCLUDE_CPU_HH_
