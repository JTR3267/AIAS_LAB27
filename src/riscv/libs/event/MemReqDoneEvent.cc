#include "event/MemReqDoneEvent.hh"

#include "DataMemory.hh"

MemReqDoneEvent::MemReqDoneEvent(DataMemory* _callee) : acalsim::SimEvent("MemReqDoneEvent"), callee(_callee) {}

void MemReqDoneEvent::renew(DataMemory* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void MemReqDoneEvent::process() { this->callee->triggerNextReq(); }
