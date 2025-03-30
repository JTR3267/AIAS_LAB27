#ifndef HW2_INCLUDE_EVENT_DMATRIGGERTRANSFEREVENT_HH_
#define HW2_INCLUDE_EVENT_DMATRIGGERTRANSFEREVENT_HH_

#include "ACALSim.hh"

class DMA;

class DMATriggerTransferEvent : public acalsim::SimEvent {
public:
	DMATriggerTransferEvent() = default;
	DMATriggerTransferEvent(DMA* _callee);
	virtual ~DMATriggerTransferEvent() = default;

	void renew(DMA* _callee);
	void process() override;

private:
	DMA* callee;
};

#endif
