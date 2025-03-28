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

#ifndef SRC_RISCV_INCLUDE_WBSTAGE_HH_
#define SRC_RISCV_INCLUDE_WBSTAGE_HH_

#include <string>

#include "ACALSim.hh"
#include "InstPacket.hh"

class WBStage : public acalsim::CPPSimBase {
public:
	WBStage(std::string name) : acalsim::CPPSimBase(name) {}
	~WBStage() {}

	void init() override {}

	void step() override {
		if (this->getPipeRegister("prMEM2WB-out")->isValid()) {
			SimPacket* pkt = this->getPipeRegister("prMEM2WB-out")->pop();
			this->accept(top->getGlobalTick(), *pkt);
			CLASS_INFO << "   WBStage step() pop an InstPacket @PC=" << ((InstPacket*)pkt)->pc;
		}
	}

	void cleanup() override {}

	void instPacketHandler(Tick when, InstPacket* pkt) {
		CLASS_INFO << "   WBStage::instPacketHandler(()  has received from prMEM2WB-out and recycled inst@PC="
		           << pkt->pc;
		acalsim::top->getRecycleContainer()->recycle(pkt);
	}

private:
};

#endif  // SRC_RISCV_INCLUDE_WBSTAGE_HH_
