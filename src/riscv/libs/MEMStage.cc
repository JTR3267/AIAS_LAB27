#include "MEMStage.hh"

void MEMStage::step() {
	InstPacket* inboundPacket = nullptr;

	if (this->getPipeRegister("prEXE2MEM-out")->isValid()) {
		InstPacket* instPacket = ((InstPacket*)this->getPipeRegister("prEXE2MEM-out")->value());
		inboundPacket          = instPacket;
	}

	if (inboundPacket)
		CLASS_INFO << "  MEMStage step() an InstPacket @PC=" << inboundPacket->pc;
	else
		CLASS_INFO << "  MEMStage step(), no inbound packet";

	if (this->getPipeRegister("prEXE2MEM-out")->isValid() && !this->getPipeRegister("prMEM2WB-in")->isStalled()) {
		SimPacket* pkt = this->getPipeRegister("prEXE2MEM-out")->pop();
		this->accept(top->getGlobalTick(), *pkt);
	}
}

void MEMStage::instPacketHandler(Tick when, SimPacket* pkt) {
	CLASS_INFO << "  MEMStage::instPacketHandler()  has received and an InstPacket @PC=" << ((InstPacket*)pkt)->pc
	           << " from prEXE2MEM-out and push it to prMEM2WB-in";

	if (!this->getPipeRegister("prMEM2WB-in")->push(pkt)) { CLASS_ERROR << "MEMStage failed to handle an InstPacket!"; }
}