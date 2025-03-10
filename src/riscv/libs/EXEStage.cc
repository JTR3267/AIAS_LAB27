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

#include "EXEStage.hh"

void EXEStage::step() {
	// Only move forward when
	// 1. the incoming slave port has instruction ready
	// 2. the downstream pipeline register is available
	Tick currTick = top->getGlobalTick();

	InstPacket* inboundPacket = nullptr;

	// check hazards
	bool controlHazard = false;
	if (this->getPipeRegister("prIF2EXE-out")->isValid()) {
		InstPacket* instPacket = ((InstPacket*)this->getPipeRegister("prIF2EXE-out")->value());
		controlHazard          = instPacket->isTakenBranch;
		inboundPacket          = instPacket;
	}

	if (inboundPacket)
		CLASS_INFO << "   EXEStage step() an InstPacket @PC=" << inboundPacket->pc
		           << " controlHazard: " << (controlHazard ? "Yes" : "No");
	else
		CLASS_INFO << "   EXEStage step(), no inbound packet";

	if (this->getPipeRegister("prIF2EXE-out")->isValid() && !this->getPipeRegister("prEXE2WB-in")->isStalled()) {
		SimPacket* pkt = this->getPipeRegister("prIF2EXE-out")->pop();
		// process tht packet regardless whether it has control hazard or not
		this->accept(currTick, *pkt);
	}
}

void EXEStage::instPacketHandler(Tick when, SimPacket* pkt) {
	CLASS_INFO << "   EXEStage::instPacketHandler()  has received and an InstPacket @PC=" << ((InstPacket*)pkt)->pc
	           << " from prIF2EXE-out and push it to prEXE2WB-in";

	// push to the prEXE2WB register
	if (!this->getPipeRegister("prEXE2WB-in")->push(pkt)) { CLASS_ERROR << "EXEStage failed to handle an InstPacket!"; }
	WBInstPacket = (InstPacket*)pkt;
}
