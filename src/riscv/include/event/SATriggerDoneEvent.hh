#ifndef HW2_INCLUDE_EVENT_SATRIGGERDONEEVENT_HH_
#define HW2_INCLUDE_EVENT_SATRIGGERDONEEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SATriggerDoneEvent : public acalsim::SimEvent {
public:
	SATriggerDoneEvent() = default;
	SATriggerDoneEvent(SystolicArray* _callee);
	virtual ~SATriggerDoneEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
