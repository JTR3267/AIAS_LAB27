#include "event/SAStepEvent.hh"

#include "SystolicArray.hh"

SAStepEvent::SAStepEvent(SystolicArray* _callee) : acalsim::SimEvent("SAStepEvent"), callee(_callee) {}

void SAStepEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SAStepEvent::process() { this->callee->systolicArrayStep(); }
