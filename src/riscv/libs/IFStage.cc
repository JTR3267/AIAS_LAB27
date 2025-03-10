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

#include "IFStage.hh"

void IFStage::step() {
	// Only move forward when
	// 1. the incoming slave port has instruction ready
	// 2. the downstream pipeline register is available

	// check hazards
	bool dataHazard = false;
	if (this->getSlavePort("soc-s")->isPopValid()) {
		InstPacket* instPacket = ((InstPacket*)this->getSlavePort("soc-s")->front());

		// IF, EXE hazard
		dataHazard =
		    EXEInstPacket && (instPacket->inst.a2.type == OPTYPE_REG && EXEInstPacket->inst.a1.type == OPTYPE_REG &&
		                          instPacket->inst.a2.reg == EXEInstPacket->inst.a1.reg ||
		                      instPacket->inst.a3.type == OPTYPE_REG && EXEInstPacket->inst.a1.type == OPTYPE_REG &&
		                          instPacket->inst.a3.reg == EXEInstPacket->inst.a1.reg);

		// IF, WB hazard
		dataHazard |=
		    WBInstPacket && (instPacket->inst.a2.type == OPTYPE_REG && WBInstPacket->inst.a1.type == OPTYPE_REG &&
		                         instPacket->inst.a2.reg == WBInstPacket->inst.a1.reg ||
		                     instPacket->inst.a3.type == OPTYPE_REG && WBInstPacket->inst.a1.type == OPTYPE_REG &&
		                         instPacket->inst.a3.reg == WBInstPacket->inst.a1.reg);
	}
	bool controlHazard = false;
	if (EXEInstPacket) { controlHazard = EXEInstPacket->isTakenBranch; }

	Tick currTick = top->getGlobalTick();
	if (this->getSlavePort("soc-s")->isPopValid()) {
		CLASS_INFO << "   IFStage step() : has an inbound  InstPacket availble ";

		if (!dataHazard && !controlHazard) {
			CLASS_INFO << "   IFStage step() :  popped an InstPacket";
			SimPacket* pkt = this->getSlavePort("soc-s")->pop();
			this->accept(currTick, *pkt);

		} else {
			WBInstPacket  = EXEInstPacket;
			EXEInstPacket = nullptr;
			// There are still pending request but no new input in the next cycle
			this->forceStepInNextIteration();
			if (dataHazard) CLASS_INFO << "   IFStage step() :  data Hazard detected. Stall IFStage";
			if (controlHazard) CLASS_INFO << "   IFStage step() :  control Hazard detected. Stall IFStage";
		}
	}
}

void IFStage::instPacketHandler(Tick when, SimPacket* pkt) {
	CLASS_INFO << "   IFStage::instPacketHandler() has received InstPacket @PC=" << ((InstPacket*)pkt)->pc
	           << " from soc-s and push it to prIF2EXE-in";

	// push to the prIF2EXE register
	if (!this->getPipeRegister("prIF2EXE-in")->push(pkt)) { CLASS_ERROR << "IFStage failed to handle an InstPacket!"; }
	WBInstPacket  = EXEInstPacket;
	EXEInstPacket = (InstPacket*)pkt;
}
