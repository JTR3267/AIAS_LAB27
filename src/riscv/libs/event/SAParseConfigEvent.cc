#include "event/SAParseConfigEvent.hh"

#include "SystolicArray.hh"

SAParseConfigEvent::SAParseConfigEvent(SystolicArray* _callee)
    : acalsim::SimEvent("SAParseConfigEvent"), callee(_callee) {}

void SAParseConfigEvent::renew(SystolicArray* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void SAParseConfigEvent::process() { this->callee->saParseConfig(); }
