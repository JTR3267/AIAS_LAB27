#include "event/DMAReadRespEvent.hh"

#include "DMA.hh"

DMAReadRespEvent::DMAReadRespEvent(DMA* _callee, MemReadRespPacket* _memRespPkt)
    : acalsim::SimEvent("DMAReadRespEvent"), callee(_callee), memRespPkt(_memRespPkt) {}

void DMAReadRespEvent::renew(DMA* _callee, MemReadRespPacket* _memRespPkt) {
	this->acalsim::SimEvent::renew();
	this->callee     = _callee;
	this->memRespPkt = _memRespPkt;
}

void DMAReadRespEvent::process() { this->callee->sendReadResp(this->memRespPkt); }
