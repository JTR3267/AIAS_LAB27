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

#ifndef HW2_INCLUDE_DATAMEMORY_HH_
#define HW2_INCLUDE_DATAMEMORY_HH_

#include <queue>
#include <string>

#include "ACALSim.hh"
#include "BaseMemory.hh"
#include "DataStruct.hh"
#include "MemPacket.hh"

typedef struct MemReqInfo {
	acalsim::SimPacket* memReqPkt;
	int                 validBytes;
	uint32_t            data;
} MemReqInfo;

/**
 * @class DataMemory
 * @brief Data memory module for the system
 * @details Implements a memory module that handles both read and write operations
 *          Inherits from SimModule for simulation functionality and BaseMemory
 *          for basic memory operations
 */
class DataMemory : public acalsim::CPPSimBase, public BaseMemory {
public:
	/**
	 * @brief Constructor for DataMemory
	 * @param _name Name identifier for the memory module
	 * @param _size Size of the memory in bytes
	 */
	DataMemory(std::string _name, size_t _size) : acalsim::CPPSimBase(_name), BaseMemory(_size) {}

	/**
	 * @brief Virtual destructor
	 */
	virtual ~DataMemory() {}

	/**
	 * @brief Handles memory read request packets
	 * @param _when Simulation time tick when the request was received
	 * @param _memReqPkt Pointer to the memory read request packet
	 * @details Processes incoming read requests and generates appropriate responses
	 */
	void memReadReqHandler(MemReadReqPacket* _memReadReqPkt);

	/**
	 * @brief Handles memory write request packets
	 * @param _when Simulation time tick when the request was received
	 * @param _memReqPkt Pointer to the memory write request packet
	 * @details Processes incoming write requests and updates memory contents
	 */
	void memWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, int validBytes, uint32_t data);

	void memReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt);

	void triggerNextReq();

	void dumpMemory(uint32_t start, uint32_t end, std::string file_path);

	void sendReadResp(MemReadRespPacket* readRespPkt);

	void step() final;

	void cleanup() final;

private:
	std::queue<MemReqInfo> pending_req_queue;
	MemWriteReqPacket*     unmatchWriteReq  = nullptr;
	MemWriteDataPacket*    unmatchWriteData = nullptr;
	bool                   is_idle          = true;
};

#endif
