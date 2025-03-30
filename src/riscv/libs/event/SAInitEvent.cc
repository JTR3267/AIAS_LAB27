#include "event/SAInitEvent.hh"

#include "SystolicArray.hh"

SAInitEvent::SAInitEvent(SystolicArray* _callee) : acalsim::SimEvent("SAInitEvent"), callee(_callee) {}

void SAInitEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SAInitEvent::process() { this->callee->initSystolicArray(); }
