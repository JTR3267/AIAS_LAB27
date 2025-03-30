#ifndef HW2_INCLUDE_EVENT_SASTEPEVENT_HH_
#define HW2_INCLUDE_EVENT_SASTEPEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SAStepEvent : public acalsim::SimEvent {
public:
	SAStepEvent() = default;
	SAStepEvent(SystolicArray* _callee);
	virtual ~SAStepEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
