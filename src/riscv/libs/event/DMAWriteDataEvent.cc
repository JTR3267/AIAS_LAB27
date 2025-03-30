#include "event/DMAWriteDataEvent.hh"

#include "DMA.hh"

DMAWriteDataEvent::DMAWriteDataEvent(DMA* _callee, MemWriteDataPacket* _memReqPkt)
    : acalsim::SimEvent("DMAWriteDataEvent"), callee(_callee), memReqPkt(_memReqPkt) {}

void DMAWriteDataEvent::renew(DMA* _callee, MemWriteDataPacket* _memReqPkt) {
	this->acalsim::SimEvent::renew();
	this->callee    = _callee;
	this->memReqPkt = _memReqPkt;
}

void DMAWriteDataEvent::process() { this->callee->sendWriteData(this->memReqPkt); }
