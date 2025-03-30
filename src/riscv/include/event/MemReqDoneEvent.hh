#ifndef HW2_INCLUDE_EVENT_MEMREQDONEEVENT_HH_
#define HW2_INCLUDE_EVENT_MEMREQDONEEVENT_HH_

#include "ACALSim.hh"

class DataMemory;

class MemReqDoneEvent : public acalsim::SimEvent {
public:
	MemReqDoneEvent() = default;
	MemReqDoneEvent(DataMemory* _callee);
	virtual ~MemReqDoneEvent() = default;

	void renew(DataMemory* _callee);
	void process() override;

private:
	DataMemory* callee;
};

#endif
