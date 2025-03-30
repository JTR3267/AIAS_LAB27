#ifndef HW2_INCLUDE_EVENT_DMAWRITEDATAEVENT_HH_
#define HW2_INCLUDE_EVENT_DMAWRITEDATAEVENT_HH_

#include "ACALSim.hh"

class DMA;
class MemWriteDataPacket;

class DMAWriteDataEvent : public acalsim::SimEvent {
public:
	DMAWriteDataEvent() = default;
	DMAWriteDataEvent(DMA* _callee, MemWriteDataPacket* _memReqPkt);
	virtual ~DMAWriteDataEvent() = default;

	void renew(DMA* _callee, MemWriteDataPacket* _memReqPkt);
	void process() override;

private:
	DMA*                callee;
	MemWriteDataPacket* memReqPkt;
};

#endif
