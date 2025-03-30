#include "event/SAReadRespEvent.hh"

#include "SystolicArray.hh"

SAReadRespEvent::SAReadRespEvent(SystolicArray* _callee, MemReadRespPacket* _memRespPkt)
    : acalsim::SimEvent("SAReadRespEvent"), callee(_callee), memRespPkt(_memRespPkt) {}

void SAReadRespEvent::renew(SystolicArray* _callee, MemReadRespPacket* _memRespPkt) {
	this->acalsim::SimEvent::renew();
	this->callee     = _callee;
	this->memRespPkt = _memRespPkt;
}

void SAReadRespEvent::process() { this->callee->sendReadResp(this->memRespPkt); }
