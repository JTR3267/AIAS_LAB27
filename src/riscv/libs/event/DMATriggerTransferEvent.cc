#include "event/DMATriggerTransferEvent.hh"

#include "DMA.hh"

DMATriggerTransferEvent::DMATriggerTransferEvent(DMA* _callee)
    : acalsim::SimEvent("DMATriggerTransferEvent"), callee(_callee) {}

void DMATriggerTransferEvent::renew(DMA* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void DMATriggerTransferEvent::process() { this->callee->triggerDmaTransfer(); }
