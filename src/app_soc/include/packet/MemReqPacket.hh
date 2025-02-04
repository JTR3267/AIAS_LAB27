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

#ifndef SRC_APP_SOC_INCLUDE_MEMREQPACKET_HH_
#define SRC_APP_SOC_INCLUDE_MEMREQPACKET_HH_

#include <string>

#include "ACALSim.hh"

typedef struct {
} request;

class MemReqPacket : public acalsim::SimPacket {
public:
	enum class ReqType { READ, WRITE };

	MemReqPacket(const std::string& _name, const uint32_t& _addr, const uint32_t& _data, const ReqType& _type) {}
	~MemReqPacket() {}

	void visit(acalsim::Tick _when, acalsim::SimModule& _module) override;
	void visit(acalsim::Tick _when, acalsim::SimBase& _simulator) override;

private:
	uint32_t addr;
	uint32_t data;
	ReqType  type;
};

#endif  // SRC_APP_SOC_INCLUDE_MEMREQPACKET_HH_
