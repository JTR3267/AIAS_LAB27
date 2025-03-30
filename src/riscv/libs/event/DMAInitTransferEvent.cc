#include "event/DMAInitTransferEvent.hh"

#include "DMA.hh"

DMAInitTransferEvent::DMAInitTransferEvent(DMA* _callee) : acalsim::SimEvent("DMAInitTransferEvent"), callee(_callee) {}

void DMAInitTransferEvent::renew(DMA* _callee) {
	this->acalsim::SimEvent::renew();
	this->callee = _callee;
}

void DMAInitTransferEvent::process() { this->callee->initiateDmaTransfer(); }
