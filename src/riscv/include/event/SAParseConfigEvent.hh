#ifndef HW2_INCLUDE_EVENT_SAPARSECONFIGEVENT_HH_
#define HW2_INCLUDE_EVENT_SAPARSECONFIGEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SAParseConfigEvent : public acalsim::SimEvent {
public:
	SAParseConfigEvent() = default;
	SAParseConfigEvent(SystolicArray* _callee);
	virtual ~SAParseConfigEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
