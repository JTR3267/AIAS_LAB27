#include "event/SAReqDoneEvent.hh"

#include "SystolicArray.hh"

SAReqDoneEvent::SAReqDoneEvent(SystolicArray* _callee) : acalsim::SimEvent("SAReqDoneEvent"), callee(_callee) {}

void SAReqDoneEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SAReqDoneEvent::process() { this->callee->triggerNextReq(); }
