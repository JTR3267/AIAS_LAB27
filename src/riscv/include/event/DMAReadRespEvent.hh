#ifndef HW2_INCLUDE_EVENT_DMAREADRESPEVENT_HH_
#define HW2_INCLUDE_EVENT_DMAREADRESPEVENT_HH_

#include "ACALSim.hh"

class DMA;
class MemReadRespPacket;

class DMAReadRespEvent : public acalsim::SimEvent {
public:
	DMAReadRespEvent() = default;
	DMAReadRespEvent(DMA* _callee, MemReadRespPacket* _memRespPkt);
	virtual ~DMAReadRespEvent() = default;

	void renew(DMA* _callee, MemReadRespPacket* _memRespPkt);
	void process() override;

private:
	DMA*               callee;
	MemReadRespPacket* memRespPkt;
};

#endif
