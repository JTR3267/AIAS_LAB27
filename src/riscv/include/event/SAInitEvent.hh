#ifndef HW2_INCLUDE_EVENT_SAINITEVENT_HH_
#define HW2_INCLUDE_EVENT_SAINITEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;

class SAInitEvent : public acalsim::SimEvent {
public:
	SAInitEvent() = default;
	SAInitEvent(SystolicArray* _callee);
	virtual ~SAInitEvent() = default;

	void renew(SystolicArray* _callee);
	void process() override;

private:
	SystolicArray* callee;
};

#endif
