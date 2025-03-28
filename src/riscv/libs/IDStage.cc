#include "IDStage.hh"

#include "InstrParse.hh"

void IDStage::step() {
	if (branchStall) {
		CLASS_INFO << "   IDStage step() : branchStall is true, skip this step";

		this->forceStepInNextIteration();

		WBInstPacket  = MEMInstPacket;
		MEMInstPacket = EXEInstPacket;
		EXEInstPacket = nullptr;
		branchStall   = false;
		return;
	}

	// check dataHazard
	bool dataHazard = false;
	if (this->getPipeRegister("prIF2ID-out")->isValid()) {
		InstPacket* instPacket = ((InstPacket*)this->getPipeRegister("prIF2ID-out")->value());
		// ID, EXE hazard
		auto EXEDestReg = EXEInstPacket ? getDestReg(EXEInstPacket->inst) : 0;
		// ID, MEM hazard
		auto MEMDestReg = MEMInstPacket ? getDestReg(MEMInstPacket->inst) : 0;
		// ID, WB hazard
		auto WBDestReg = WBInstPacket ? getDestReg(WBInstPacket->inst) : 0;

		if (instPacket)
			dataHazard = (EXEInstPacket && checkDataHazard(EXEDestReg, instPacket->inst)) ||
			             (MEMInstPacket && checkDataHazard(MEMDestReg, instPacket->inst)) ||
			             (WBInstPacket && checkDataHazard(WBDestReg, instPacket->inst));
	}

	// check controlHazard
	bool controlHazard = false;
	if (EXEInstPacket) { controlHazard = EXEInstPacket->isTakenBranch; }

	if (this->getPipeRegister("prIF2ID-out")->isValid()) {
		CLASS_INFO << "   IDStage step() : has an inbound  InstPacket availble ";

		if (!dataHazard && !controlHazard) {
			CLASS_INFO << "   IDStage step() :  popped an InstPacket";
			SimPacket* pkt = this->getPipeRegister("prIF2ID-out")->pop();
			this->accept(top->getGlobalTick(), *pkt);
		} else {
			if (dataHazard) CLASS_INFO << "   IDStage step() :  data Hazard detected. Stall IDStage";
			if (controlHazard) {
				CLASS_INFO << "   IDStage step() :  control Hazard detected. Stall IDStage";
				branchStall = true;
			}

			this->forceStepInNextIteration();

			WBInstPacket  = MEMInstPacket;
			MEMInstPacket = EXEInstPacket;
			EXEInstPacket = nullptr;
		}
	}
}

void IDStage::instPacketHandler(Tick when, SimPacket* pkt) {
	CLASS_INFO << "   IDStage::instPacketHandler() has received InstPacket @PC=" << ((InstPacket*)pkt)->pc
	           << " from prIF2ID-out and push it to prID2EXE-in";

	if (!this->getPipeRegister("prID2EXE-in")->push(pkt)) { CLASS_ERROR << "IDStage failed to handle an InstPacket!"; }

	WBInstPacket  = MEMInstPacket;
	MEMInstPacket = EXEInstPacket;
	EXEInstPacket = (InstPacket*)pkt;
}
