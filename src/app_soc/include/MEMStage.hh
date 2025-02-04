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

#ifndef SRC_APP_SOC_INCLUDE_MEMSTAGE_HH_
#define SRC_APP_SOC_INCLUDE_MEMSTAGE_HH_

#include <string>

#include "ACALSim.hh"
#include "CPUDefs.hh"
#include "MemRespPacket.hh"
#include "Register.hh"

/**
 * @brief A class representing a component template.
 *
 * @details The class inherits from SimBase and provides the basic structure for defining a component.
 */
class MEMStage : public acalsim::SimModule {
public:
	enum class mem_stage_status { WAIT, IDLE };

	/**
	 * @brief Constructor for the Template class.
	 *
	 * @param name The name of the component.
	 */
	MEMStage(const std::string& _name, const Register<mem_stage_info>* _exe_mem_reg,
	         const Register<wb_stage_info>* _mem_wb_reg)
	    : acalsim::SimModule(_name), exe_mem_reg(_exe_mem_reg), mem_wb_reg(_mem_wb_reg){};

	~MEMStage();

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

	/**
	 * Update the outbound results according to the inbound register
	 * of each stage itself.
	 */
	void execDataPath();

	void setRespPkt(MemRespPkt* _resp_pkt) { this->resp_pkt = _resp_pkt; }

	void processRespPkt(const uint32_t& _data);

	void setStatus(mem_stage_status _status) { this->status = _status; }

	void checkMemoryAccess(const mem_stage_info* _info);

	bool checkDataHazard(int _rs1, int _rs2);

	void sendReqToMemory(const SimPacket* _pkt);

private:
	Register<exe_stage_out>* exe_mem_reg;
	Register<mem_stage_out>* mem_wb_reg;
	MemRespPkt*              resp_pkt;
	mem_stage_status         status = mem_stage_status::IDLE;
};

#endif  // SRC_APP_SOC_INCLUDE_MEMSTAGE_HH_
