#include "DMA.hh"

#include "event/DMAInitTransferEvent.hh"
#include "event/DMAReadRespEvent.hh"
#include "event/DMAReqDoneEvent.hh"
#include "event/DMATriggerTransferEvent.hh"
#include "event/DMAWriteDataEvent.hh"
#include "event/DMAWriteReqEvent.hh"

DMA::DMA(std::string _name, int _base_addr, int _size)
    : acalsim::CPPSimBase(_name), base_addr(_base_addr), size(_size) {
	for (int i = 0; i < 6; i++) { this->rf[i] = 0; }
}

void DMA::step() {
	for (auto s_port : this->s_ports_) {
		if (s_port.second->isPopValid()) {
			auto packet = s_port.second->pop();
			this->accept(acalsim::top->getGlobalTick(), *packet);
		}
	}
}

void DMA::triggerNextReq() {
	if (!this->pending_req_queue.empty()) {
		auto pkt = this->pending_req_queue.front().memReqPkt;
		if (auto readPkt = dynamic_cast<MemReadReqPacket*>(pkt)) {
			dmaReadReqHandler(readPkt);
		} else if (auto writePkt = dynamic_cast<MemWriteReqPacket*>(pkt)) {
			dmaWriteReqHandler(writePkt, this->pending_req_queue.front().destAddr);
		}
		this->pending_req_queue.pop();
	} else {
		this->is_idle = true;
	}
}

void DMA::dmaReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt) {
	if (auto pkt = dynamic_cast<MemReadReqPacket*>(_memReqPkt)) {
		this->pending_req_queue.push({pkt, 0, 0});
	} else if (auto pkt = dynamic_cast<MemWriteReqPacket*>(_memReqPkt)) {
		if (this->unmatchWriteData != nullptr) {
			this->pending_req_queue.push({pkt, 0, this->unmatchWriteData->getData()});
			acalsim::top->getRecycleContainer()->recycle(this->unmatchWriteData);
			this->unmatchWriteData = nullptr;
		} else {
			this->unmatchWriteReq = pkt;
		}
	} else if (auto pkt = dynamic_cast<MemWriteDataPacket*>(_memReqPkt)) {
		if (this->unmatchWriteReq != nullptr) {
			this->pending_req_queue.push({this->unmatchWriteReq, 0, pkt->getData()});
			acalsim::top->getRecycleContainer()->recycle(pkt);
			this->unmatchWriteReq = nullptr;
		} else {
			this->unmatchWriteData = pkt;
		}
	}

	if (this->is_idle) {
		this->is_idle = false;
		triggerNextReq();
	}
}

void DMA::dmaReadReqHandler(MemReadReqPacket* _memReadReqPkt) {
	instr      i       = _memReadReqPkt->getInstr();
	instr_type op      = _memReadReqPkt->getOP();
	uint32_t   addr    = _memReadReqPkt->getAddr();
	operand    a1      = _memReadReqPkt->getA1();
	size_t     srcIdx  = _memReadReqPkt->getSrcIdx();
	size_t     destIdx = _memReadReqPkt->getDstIdx();

	size_t   bytes = 0;
	uint32_t ret   = 0;

	switch (op) {
		case LB:
		case LBU: bytes = 1; break;
		case LH:
		case LHU: bytes = 2; break;
		case LW: bytes = 4; break;
	}

	ASSERT_MSG(addr + bytes <= this->base_addr + this->size, "The DMA memory region is out of range.");
	void* data = (uint8_t*)this->rf + (addr - this->base_addr);

	switch (op) {
		case LB: ret = static_cast<uint32_t>(*(int8_t*)data); break;
		case LBU: ret = *(uint8_t*)data; break;
		case LH: ret = static_cast<uint32_t>(*(int16_t*)data); break;
		case LHU: ret = *(uint16_t*)data; break;
		case LW: ret = *(uint32_t*)data; break;
	}

	auto               rc = acalsim::top->getRecycleContainer();
	MemReadRespPacket* memRespPkt =
	    rc->acquire<MemReadRespPacket>(&MemReadRespPacket::renew, i, op, ret, a1, destIdx, srcIdx);
	DMAReadRespEvent* event = rc->acquire<DMAReadRespEvent>(&DMAReadRespEvent::renew, this, memRespPkt);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);

	DMAReqDoneEvent* dmaReqDoneEvent = rc->acquire<DMAReqDoneEvent>(&DMAReqDoneEvent::renew, this);
	this->scheduleEvent(dmaReqDoneEvent, acalsim::top->getGlobalTick() + 1);
	rc->recycle(_memReadReqPkt);
}

void DMA::dmaWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, uint32_t data) {
	instr_type op   = _memWriteReqPkt->getOP();
	uint32_t   addr = _memWriteReqPkt->getAddr();
	size_t     bytes;

	switch (op) {
		case SB: bytes = 1; break;
		case SH: bytes = 2; break;
		case SW: bytes = 4; break;
	}

	ASSERT_MSG(addr + bytes <= this->base_addr + this->size, "The DMA memory region is out of range.");

	switch (op) {
		case SB: {
			uint8_t val8 = static_cast<uint8_t>(data);
			std::memcpy((uint8_t*)this->rf + addr - this->base_addr, &val8, sizeof(uint8_t));
			break;
		}
		case SH: {
			uint16_t val16 = static_cast<uint16_t>(data);
			std::memcpy((uint8_t*)this->rf + addr - this->base_addr, &val16, sizeof(uint16_t));
			break;
		}
		case SW: {
			uint32_t val32 = static_cast<uint32_t>(data);
			std::memcpy((uint8_t*)this->rf + addr - this->base_addr, &val32, sizeof(uint32_t));
			break;
		}
	}

	auto             rc              = acalsim::top->getRecycleContainer();
	DMAReqDoneEvent* dmaReqDoneEvent = rc->acquire<DMAReqDoneEvent>(&DMAReqDoneEvent::renew, this);
	this->scheduleEvent(dmaReqDoneEvent, acalsim::top->getGlobalTick() + 1);

	if (this->rf[0] & 1) {
		DMAInitTransferEvent* dmaInitTransferEvent =
		    rc->acquire<DMAInitTransferEvent>(&DMAInitTransferEvent::renew, this);
		this->scheduleEvent(dmaInitTransferEvent, acalsim::top->getGlobalTick() + 1);
	}
	rc->recycle(_memWriteReqPkt);
}

void DMA::dmaReadRespHandler(acalsim::Tick _when, MemReadRespPacket* _memReadRespPkt) {
	auto     rc   = acalsim::top->getRecycleContainer();
	uint32_t data = _memReadRespPkt->getData();
	instr    ins;

	MemWriteReqPacket*  pkt;
	MemWriteDataPacket* dataPkt;

	auto accelRegBaseAddr = acalsim::top->getParameter<int>("SOC", "accel_reg_base_addr");
	auto accelRegSize     = acalsim::top->getParameter<int>("SOC", "accel_reg_size");
	auto accelBufBaseAddr = acalsim::top->getParameter<int>("SOC", "accel_buf_base_addr");
	auto accelBufSize     = acalsim::top->getParameter<int>("SOC", "accel_buf_size");

	if ((this->curDestAddr >= accelRegBaseAddr && this->curDestAddr < accelRegBaseAddr + accelRegSize) ||
	    (this->curDestAddr >= accelBufBaseAddr && this->curDestAddr < accelBufBaseAddr + accelBufSize)) {
		pkt     = rc->acquire<MemWriteReqPacket>(&MemWriteReqPacket::renew, ins, SW, this->curDestAddr, 4, 7);
		dataPkt = rc->acquire<MemWriteDataPacket>(&MemWriteDataPacket::renew, data, this->curBytes, 5, 8);
	} else {
		pkt     = rc->acquire<MemWriteReqPacket>(&MemWriteReqPacket::renew, ins, SW, this->curDestAddr, 4, 1);
		dataPkt = rc->acquire<MemWriteDataPacket>(&MemWriteDataPacket::renew, data, this->curBytes, 5, 2);
	}

	DMAWriteReqEvent* wrEvent = rc->acquire<DMAWriteReqEvent>(&DMAWriteReqEvent::renew, this, pkt);
	this->scheduleEvent(wrEvent, acalsim::top->getGlobalTick() + 1);

	DMAWriteDataEvent* wdEvent = rc->acquire<DMAWriteDataEvent>(&DMAWriteDataEvent::renew, this, dataPkt);
	this->scheduleEvent(wdEvent, acalsim::top->getGlobalTick() + 1);

	DMATriggerTransferEvent* event = rc->acquire<DMATriggerTransferEvent>(&DMATriggerTransferEvent::renew, this);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 2);

	rc->recycle(_memReadRespPkt);
};

void DMA::initiateDmaTransfer() {
	auto srcBaseAddr  = rf[1];
	auto destBaseAddr = rf[2];
	auto tensorHeight = (rf[3] & 0xff) + 1;
	auto tensorWidth  = ((rf[3] >> 8) & 0xff) + 1;
	auto destStride   = (rf[3] >> 16) & 0xff;
	auto srcStride    = (rf[3] >> 24) & 0xff;

	if (tensorHeight > 1) {
		ASSERT_MSG(srcStride >= tensorWidth, "Wrong DMA config.");
		ASSERT_MSG(destStride >= tensorWidth, "Wrong DMA config.");
	}

	auto    rc = acalsim::top->getRecycleContainer();
	int     readBytes;
	instr   ins;
	operand op;

	auto accelRegBaseAddr = acalsim::top->getParameter<int>("SOC", "accel_reg_base_addr");
	auto accelRegSize     = acalsim::top->getParameter<int>("SOC", "accel_reg_size");
	auto accelBufBaseAddr = acalsim::top->getParameter<int>("SOC", "accel_buf_base_addr");
	auto accelBufSize     = acalsim::top->getParameter<int>("SOC", "accel_buf_size");

	for (int h = 0; h < tensorHeight; h++) {
		readBytes = 0;
		while (tensorWidth - readBytes) {
			uint32_t          addr = srcBaseAddr + h * srcStride + readBytes;
			MemReadReqPacket* memReadReqPkt;

			if ((addr >= accelRegBaseAddr && addr < accelRegBaseAddr + accelRegSize) ||
			    (addr >= accelBufBaseAddr && addr < accelBufBaseAddr + accelBufSize)) {
				memReadReqPkt = rc->acquire<MemReadReqPacket>(&MemReadReqPacket::renew, ins, LW, addr, op, 3, 6);
			} else {
				memReadReqPkt = rc->acquire<MemReadReqPacket>(&MemReadReqPacket::renew, ins, LW, addr, op, 3, 0);
			}

			int validBytes = tensorWidth - readBytes >= 4 ? 4 : tensorWidth - readBytes;
			this->dma_req_queue.push({memReadReqPkt, validBytes, destBaseAddr + h * destStride + readBytes});
			readBytes += validBytes;
		}
	}

	DMATriggerTransferEvent* event = rc->acquire<DMATriggerTransferEvent>(&DMATriggerTransferEvent::renew, this);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
}

void DMA::triggerDmaTransfer() {
	auto rc = acalsim::top->getRecycleContainer();

	if (!this->dma_req_queue.empty()) {
		auto dmaInfo      = this->dma_req_queue.front();
		this->curDestAddr = dmaInfo.destAddr;
		this->curBytes    = dmaInfo.bytes;

		this->sendReadReq(static_cast<MemReadReqPacket*>(dmaInfo.memReqPkt));

		this->dma_req_queue.pop();
	} else {
		this->rf[0] &= ~1;
		this->rf[5] |= 1;
	}
}

void DMA::sendReadResp(MemReadRespPacket* _memRespPkt) {
	if (!this->getPipeRegister("dma2bus-resp-m")->isStalled()) {
		if (!this->getPipeRegister("dma2bus-resp-m")->push(_memRespPkt)) {
			CLASS_ERROR << "Cant push read resp from dma to bus";
		}
	} else {
		CLASS_ERROR << "DMA read response is stalled";
	}
}

void DMA::sendReadReq(MemReadReqPacket* _memReadReqPkt) {
	if (!this->getPipeRegister("dma2bus-rr-m")->isStalled()) {
		if (!this->getPipeRegister("dma2bus-rr-m")->push(_memReadReqPkt)) {
			CLASS_ERROR << "Cant push read req from dma to bus";
		}
	} else {
		CLASS_ERROR << "DMA read request is stalled";
	}
}

void DMA::sendWriteReq(MemWriteReqPacket* _memWriteReqPkt) {
	if (!this->getPipeRegister("dma2bus-wr-m")->isStalled()) {
		if (!this->getPipeRegister("dma2bus-wr-m")->push(_memWriteReqPkt)) {
			CLASS_ERROR << "Cant push write req from dma to bus";
		}
	} else {
		CLASS_ERROR << "DMA write request is stalled";
	}
}

void DMA::sendWriteData(MemWriteDataPacket* _memWriteDataPkt) {
	if (!this->getPipeRegister("dma2bus-wd-m")->isStalled()) {
		if (!this->getPipeRegister("dma2bus-wd-m")->push(_memWriteDataPkt)) {
			CLASS_ERROR << "Cant push write data from dma to bus";
		}
	} else {
		CLASS_ERROR << "DMA write data is stalled";
	}
}
