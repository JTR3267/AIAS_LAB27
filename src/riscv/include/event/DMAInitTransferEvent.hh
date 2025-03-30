#ifndef HW2_INCLUDE_EVENT_DMAINITTRANSFEREVENT_HH_
#define HW2_INCLUDE_EVENT_DMAINITTRANSFEREVENT_HH_

#include "ACALSim.hh"

class DMA;

class DMAInitTransferEvent : public acalsim::SimEvent {
public:
	DMAInitTransferEvent() = default;
	DMAInitTransferEvent(DMA* _callee);
	virtual ~DMAInitTransferEvent() = default;

	void renew(DMA* _callee);
	void process() override;

private:
	DMA* callee;
};

#endif
