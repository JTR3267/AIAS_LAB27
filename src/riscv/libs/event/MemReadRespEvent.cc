#include "event/MemReadRespEvent.hh"

#include "DataMemory.hh"

MemReadRespEvent::MemReadRespEvent(DataMemory* _callee, MemReadRespPacket* _memRespPkt)
    : acalsim::SimEvent("MemReqDoneEvent"), callee(_callee), memRespPkt(_memRespPkt) {}

void MemReadRespEvent::renew(DataMemory* _callee, MemReadRespPacket* _memRespPkt) {
	this->acalsim::SimEvent::renew();
	this->callee     = _callee;
	this->memRespPkt = _memRespPkt;
}

void MemReadRespEvent::process() { this->callee->sendReadResp(this->memRespPkt); }
