#ifndef HW2_INCLUDE_EVENT_DMAWRITEREQEVENT_HH_
#define HW2_INCLUDE_EVENT_DMAWRITEREQEVENT_HH_

#include "ACALSim.hh"

class DMA;
class MemWriteReqPacket;

class DMAWriteReqEvent : public acalsim::SimEvent {
public:
	DMAWriteReqEvent() = default;
	DMAWriteReqEvent(DMA* _callee, MemWriteReqPacket* _memReqPkt);
	virtual ~DMAWriteReqEvent() = default;

	void renew(DMA* _callee, MemWriteReqPacket* _memReqPkt);
	void process() override;

private:
	DMA*               callee;
	MemWriteReqPacket* memReqPkt;
};

#endif
