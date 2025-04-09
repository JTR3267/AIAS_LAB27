#include "event/SATriggerDoneEvent.hh"

#include "SystolicArray.hh"

SATriggerDoneEvent::SATriggerDoneEvent(SystolicArray* _callee)
    : acalsim::SimEvent("SATriggerDoneEvent"), callee(_callee) {}

void SATriggerDoneEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SATriggerDoneEvent::process() { this->callee->saTriggerDone(); }
