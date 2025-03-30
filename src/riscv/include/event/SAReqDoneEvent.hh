#ifndef HW2_INCLUDE_EVENT_SAREQDONEEVENT_HH_
#define HW2_INCLUDE_EVENT_SAREQDONEEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SAReqDoneEvent : public acalsim::SimEvent {
public:
	SAReqDoneEvent() = default;
	SAReqDoneEvent(SystolicArray* _callee);
	virtual ~SAReqDoneEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
