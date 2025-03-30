#ifndef HW2_INCLUDE_EVENT_DMAREQDONEEVENT_HH_
#define HW2_INCLUDE_EVENT_DMAREQDONEEVENT_HH_

#include "ACALSim.hh"

class DMA;

class DMAReqDoneEvent : public acalsim::SimEvent {
public:
	DMAReqDoneEvent() = default;
	DMAReqDoneEvent(DMA* _callee);
	virtual ~DMAReqDoneEvent() = default;

	void renew(DMA* _callee);
	void process() override;

private:
	DMA* callee;
};

#endif
