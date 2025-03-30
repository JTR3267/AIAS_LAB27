#include "event/SACalDoneEvent.hh"

#include "SystolicArray.hh"

SACalDoneEvent::SACalDoneEvent(SystolicArray* _callee) : acalsim::SimEvent("SACalDoneEvent"), callee(_callee) {}

void SACalDoneEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SACalDoneEvent::process() { this->callee->systolicArrayCalDone(); }
