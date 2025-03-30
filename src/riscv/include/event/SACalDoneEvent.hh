#ifndef HW2_INCLUDE_EVENT_SACALDONEEVENT_HH_
#define HW2_INCLUDE_EVENT_SACALDONEEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SACalDoneEvent : public acalsim::SimEvent {
public:
	SACalDoneEvent() = default;
	SACalDoneEvent(SystolicArray* _callee);
	virtual ~SACalDoneEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
