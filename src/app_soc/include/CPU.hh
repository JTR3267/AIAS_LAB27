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
#include "CPUDefs.hh"
#include "DataMemory.hh"
#include "InstMemory.hh"
#include "Register.hh"
#include "packet/MemRespPacket.hh"

struct ChromeTraceData {
	bool        is_started = false;
	std::string stage_name;
	std::string event_name;
};

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
	CPU(const std::string& _name, const std::string& _m_port, const std::string& _s_port);

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

	/**
	 * Update system states according to the outbound results
	 * of all stages.
	 *
	 * 1. All stages update the inbound registers of the next stage.
	 * 2. WB updates register file with WB stage's outbound result.
	 * 3. Update PC.
	 * 4. Schedule a CpuSingleCycleEvent at the next cycle
	 *    if not all stages are set to stall or a program
	 *    termination instruction `hcf` is not detected in WB stage
	 */
	void updateSystemStates();

	void execDataPath();

	void updatePipeRegisters();

	void updateStatus();

	void updatePC();

	void checkNextCycleEvent();

	void recordTrace(uint32_t pc, std::string inst_name, ChromeTraceData* data);

	template <typename T>
	void recordTrace(Register<T>* reg, ChromeTraceData* data);

	std::string instrToString(instr_type op);

	const instr& fetchInstr(int index) { return this->imem->fetchInstr(index); }

	ChromeTraceData* getIFTraceData() { return this->if_trace_data; }

	acalsim::MasterPort* getMasterPort() { return this->m_port; }

	int getDestReg(const instr& _inst);

	bool checkDataHazard(int _rd, std::string _stage);

	void handler(MemRespPacket* _pkt);

	RegFile* getRegFile() { return this->regs; }

	void printPerfAnalysis();

	void printPerfCounter();

	void printPerfCounterValue(const std::string& _name) { this->counters.find(_name)->second.printCounterInfo(); }

	PerfCounter* getPerfCounter(const std::string& _name) {
		auto isExist = this->counters.find(_name);
		if (isExist != this->counters.end()) {
			return &isExist->second;
		} else {
			CLASS_ERROR << "Performance counter " << _name << " does not exist!";
			return nullptr;
		}
	}

	// Create a new performance counter
	void createPerfCounter(const std::string& _name) {
		auto isExist = this->counters.find(_name);
		if (isExist == this->counters.end()) {
			this->counters.insert(std::make_pair(_name, PerfCounter(_name)));
		} else {
			ERROR << "Performance counter " << _name << " already exists!";
		}
	}

private:
	ChromeTraceData*         if_trace_data;
	ChromeTraceData*         id_trace_data;
	ChromeTraceData*         exe_trace_data;
	ChromeTraceData*         mem_trace_data;
	ChromeTraceData*         wb_trace_data;
	InstMemory*              imem;
	RegFile*                 regs;
	acalsim::MasterPort*     m_port;
	acalsim::SlavePort*      s_port;
	Register<if_stage_out>*  if_id_reg;
	Register<id_stage_out>*  id_exe_reg;
	Register<exe_stage_out>* exe_mem_reg;
	Register<mem_stage_out>* mem_wb_reg;
	// Performance Counter
	std::unordered_map<std::string, PerfCounter> counters;
};

#endif  // SRC_APP_SOC_INCLUDE_CPU_HH_
