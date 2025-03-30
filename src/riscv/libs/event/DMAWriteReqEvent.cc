#include "event/DMAWriteReqEvent.hh"

#include "DMA.hh"

DMAWriteReqEvent::DMAWriteReqEvent(DMA* _callee, MemWriteReqPacket* _memReqPkt)
    : acalsim::SimEvent("DMAWriteReqEvent"), callee(_callee), memReqPkt(_memReqPkt) {}

void DMAWriteReqEvent::renew(DMA* _callee, MemWriteReqPacket* _memReqPkt) {
	this->acalsim::SimEvent::renew();
	this->callee    = _callee;
	this->memReqPkt = _memReqPkt;
}

void DMAWriteReqEvent::process() { this->callee->sendWriteReq(this->memReqPkt); }
