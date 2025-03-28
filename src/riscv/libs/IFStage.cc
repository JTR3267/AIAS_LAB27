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

#include "InstrParse.hh"

void IFStage::step() {
	if (branchStall) {
		CLASS_INFO << "   IFStage step() : branchStall is true, skip this step";

		this->forceStepInNextIteration();

		WBInstPacket  = MEMInstPacket;
		MEMInstPacket = EXEInstPacket;
		EXEInstPacket = nullptr;
		branchStall   = false;
		return;
	}

	// check dataHazard
	bool dataHazard = false;
	if (IDInstPacket) {
		// ID, EXE hazard
		auto EXEDestReg = EXEInstPacket ? getDestReg(EXEInstPacket->inst) : 0;
		// ID, MEM hazard
		auto MEMDestReg = MEMInstPacket ? getDestReg(MEMInstPacket->inst) : 0;
		// ID, WB hazard
		auto WBDestReg = WBInstPacket ? getDestReg(WBInstPacket->inst) : 0;

		dataHazard = (EXEInstPacket && checkDataHazard(EXEDestReg, IDInstPacket->inst)) ||
		             (MEMInstPacket && checkDataHazard(MEMDestReg, IDInstPacket->inst)) ||
		             (WBInstPacket && checkDataHazard(WBDestReg, IDInstPacket->inst));
	}

	// check controlHazard
	bool controlHazard = false;
	if (EXEInstPacket) { controlHazard = EXEInstPacket->isTakenBranch; }

	if (this->getSlavePort("soc-s")->isPopValid()) {
		CLASS_INFO << "   IFStage step() : has an inbound  InstPacket availble ";

		if (!dataHazard && !controlHazard) {
			CLASS_INFO << "   IFStage step() :  popped an InstPacket";
			SimPacket* pkt = this->getSlavePort("soc-s")->pop();
			this->accept(top->getGlobalTick(), *pkt);
		} else {
			if (dataHazard) CLASS_INFO << "   IFStage step() :  data Hazard detected. Stall IFStage";
			if (controlHazard) {
				CLASS_INFO << "   IFStage step() :  control Hazard detected. Stall IFStage";
				branchStall = true;
			}

			this->forceStepInNextIteration();

			WBInstPacket  = MEMInstPacket;
			MEMInstPacket = EXEInstPacket;
			EXEInstPacket = nullptr;
		}
	}
}

void IFStage::instPacketHandler(Tick when, SimPacket* pkt) {
	CLASS_INFO << "   IFStage::instPacketHandler() has received InstPacket @PC=" << ((InstPacket*)pkt)->pc
	           << " from soc-s and push it to prIF2ID-in";

	// push to the prIF2ID register
	if (!this->getPipeRegister("prIF2ID-in")->push(pkt)) { CLASS_ERROR << "IFStage failed to handle an InstPacket!"; }

	WBInstPacket  = MEMInstPacket;
	MEMInstPacket = EXEInstPacket;
	EXEInstPacket = IDInstPacket;
	IDInstPacket  = (InstPacket*)pkt;
}
