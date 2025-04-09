#include "SystolicArray.hh"

#include "event/SACalDoneEvent.hh"
#include "event/SAInitEvent.hh"
#include "event/SAParseConfigEvent.hh"
#include "event/SAReadRespEvent.hh"
#include "event/SAReqDoneEvent.hh"
#include "event/SAStepEvent.hh"
#include "event/SATriggerDoneEvent.hh"

SystolicArray::SystolicArray(std::string _name, u_int32_t _sa_size, int _reg_base_addr, int _reg_size,
                             int _buf_base_addr, int _buf_size)
    : acalsim::CPPSimBase(_name),
      sa_size(_sa_size),
      reg_base_addr(_reg_base_addr),
      reg_size(_reg_size),
      buf_base_addr(_buf_base_addr),
      buf_size(_buf_size) {
	for (int i = 0; i < 9; i++) { this->rf[i] = 0; }
	this->rf[8]   = 0x10101;
	this->peArray = new PE[this->sa_size * this->sa_size];
}

void SystolicArray::step() {
	for (auto s_port : this->s_ports_) {
		if (s_port.second->isPopValid()) {
			auto packet = s_port.second->pop();
			this->accept(acalsim::top->getGlobalTick(), *packet);
		}
	}
}

void SystolicArray::triggerNextReq() {
	if (!this->pending_req_queue.empty()) {
		auto pkt = this->pending_req_queue.front().memReqPkt;
		if (auto readPkt = dynamic_cast<MemReadReqPacket*>(pkt)) {
			saReadReqHandler(readPkt);
		} else if (auto writePkt = dynamic_cast<MemWriteReqPacket*>(pkt)) {
			saWriteReqHandler(writePkt, this->pending_req_queue.front().data);
		}
		this->pending_req_queue.pop();
	} else {
		this->is_idle = true;
	}
}

void SystolicArray::saReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt) {
	if (auto pkt = dynamic_cast<MemReadReqPacket*>(_memReqPkt)) {
		this->pending_req_queue.push({pkt, 0});
	} else if (auto pkt = dynamic_cast<MemWriteReqPacket*>(_memReqPkt)) {
		if (this->unmatchWriteData != nullptr) {
			this->pending_req_queue.push({pkt, this->unmatchWriteData->getData()});
			acalsim::top->getRecycleContainer()->recycle(this->unmatchWriteData);
			this->unmatchWriteData = nullptr;
		} else {
			this->unmatchWriteReq = pkt;
		}
	} else if (auto pkt = dynamic_cast<MemWriteDataPacket*>(_memReqPkt)) {
		if (this->unmatchWriteReq != nullptr) {
			this->pending_req_queue.push({this->unmatchWriteReq, pkt->getData()});
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

void SystolicArray::saReadReqHandler(MemReadReqPacket* _memReadReqPkt) {
	instr      i       = _memReadReqPkt->getInstr();
	instr_type op      = _memReadReqPkt->getOP();
	uint32_t   addr    = _memReadReqPkt->getAddr();
	operand    a1      = _memReadReqPkt->getA1();
	size_t     srcIdx  = _memReadReqPkt->getSrcIdx();
	size_t     destIdx = _memReadReqPkt->getDstIdx();

	size_t   bytes = 0;
	uint32_t ret   = 0;
	void*    data  = nullptr;

	switch (op) {
		case LB:
		case LBU: bytes = 1; break;
		case LH:
		case LHU: bytes = 2; break;
		case LW: bytes = 4; break;
	}

	if (addr >= this->buf_base_addr && addr + bytes <= this->buf_base_addr + this->buf_size) {
		data = (uint8_t*)this->buf + (addr - this->buf_base_addr);
	} else if (addr >= this->reg_base_addr && addr + bytes <= this->reg_base_addr + this->reg_size) {
		data = (uint8_t*)this->rf + (addr - this->reg_base_addr);
	} else {
		CLASS_ERROR << "The Systolic Array memory region is out of range.";
	}

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
	SAReadRespEvent* event = rc->acquire<SAReadRespEvent>(&SAReadRespEvent::renew, this, memRespPkt);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);

	SAReqDoneEvent* saReqDoneEvent = rc->acquire<SAReqDoneEvent>(&SAReqDoneEvent::renew, this);
	this->scheduleEvent(saReqDoneEvent, acalsim::top->getGlobalTick() + 1);
	rc->recycle(_memReadReqPkt);
}

void SystolicArray::saWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, uint32_t data) {
	instr_type op   = _memWriteReqPkt->getOP();
	uint32_t   addr = _memWriteReqPkt->getAddr();
	size_t     bytes;

	switch (op) {
		case SB: bytes = 1; break;
		case SH: bytes = 2; break;
		case SW: bytes = 4; break;
	}

	if (!(addr >= this->buf_base_addr && addr + bytes <= this->buf_base_addr + this->buf_size) &&
	    !(addr >= this->reg_base_addr && addr + bytes <= this->reg_base_addr + this->reg_size)) {
		CLASS_ERROR << "The Systolic Array memory region is out of range.";
	}

	switch (op) {
		case SB: {
			uint8_t val8 = static_cast<uint8_t>(data);
			if (addr >= this->buf_base_addr && addr + bytes < this->buf_base_addr + this->buf_size) {
				std::memcpy((uint8_t*)this->buf + addr - this->buf_base_addr, &val8, sizeof(uint8_t));
			} else {
				std::memcpy((uint8_t*)this->rf + addr - this->reg_base_addr, &val8, sizeof(uint8_t));
			}
			break;
		}
		case SH: {
			uint16_t val16 = static_cast<uint16_t>(data);
			if (addr >= this->buf_base_addr && addr + bytes < this->buf_base_addr + this->buf_size) {
				std::memcpy((uint8_t*)this->buf + addr - this->buf_base_addr, &val16, sizeof(uint16_t));
			} else {
				std::memcpy((uint8_t*)this->rf + addr - this->reg_base_addr, &val16, sizeof(uint16_t));
			}
			break;
		}
		case SW: {
			uint32_t val32 = static_cast<uint32_t>(data);
			if (addr >= this->buf_base_addr && addr + bytes < this->buf_base_addr + this->buf_size) {
				std::memcpy((uint8_t*)this->buf + addr - this->buf_base_addr, &val32, sizeof(uint32_t));
			} else {
				std::memcpy((uint8_t*)this->rf + addr - this->reg_base_addr, &val32, sizeof(uint32_t));
			}
			break;
		}
	}

	auto            rc             = acalsim::top->getRecycleContainer();
	SAReqDoneEvent* saReqDoneEvent = rc->acquire<SAReqDoneEvent>(&SAReqDoneEvent::renew, this);
	this->scheduleEvent(saReqDoneEvent, acalsim::top->getGlobalTick() + 1);

	if (this->rf[0] & 1) {
		SAParseConfigEvent* event = rc->acquire<SAParseConfigEvent>(&SAParseConfigEvent::renew, this);
		this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
	}

	rc->recycle(_memWriteReqPkt);
}

void SystolicArray::saParseConfig() {
	auto matrixAHeight  = (rf[2] & 0xfff) + 1;
	auto matrixAWidth   = ((rf[2] >> 12) & 0xfff) + 1;
	auto matrixBHeight  = (rf[3] & 0xfff) + 1;
	auto matrixBWidth   = ((rf[3] >> 12) & 0xfff) + 1;
	auto matrixCHeight  = (rf[4] & 0xfff) + 1;
	auto matrixCWidth   = ((rf[4] >> 12) & 0xfff) + 1;
	auto matrixAAddr    = rf[5];
	auto matrixBAddr    = rf[6];
	auto matrixCAddr    = rf[7];
	auto matrixAStride  = rf[8] & 0xff;
	auto matrixBStride  = (rf[8] >> 8) & 0xff;
	this->matrixCStride = (rf[8] >> 16) & 0xff;

	if (matrixAHeight > 1) { ASSERT_MSG(matrixAStride >= 4 * matrixAWidth, "Wrong systolic array config."); }
	if (matrixBHeight > 1) { ASSERT_MSG(matrixBStride >= 4 * matrixBWidth, "Wrong systolic array config."); }
	if (matrixCHeight > 1) { ASSERT_MSG(matrixCStride >= 4 * matrixCWidth, "Wrong systolic array config."); }
	ASSERT_MSG(matrixAWidth == matrixBHeight, "Dimension mismatch: A.width must equal B.height");
	ASSERT_MSG(matrixCHeight == matrixAHeight && matrixCWidth == matrixBWidth,
	           "Output matrix dimensions must match A.height x B.width");

	int sliceACount = (matrixAHeight + this->sa_size - 1) / this->sa_size;
	int sliceBCount = (matrixBWidth + this->sa_size - 1) / this->sa_size;

	for (int i = 0; i < sliceACount; i++) {
		auto      height  = std::min(this->sa_size, matrixAHeight - i * this->sa_size);
		uint32_t* matrixA = new uint32_t[height * matrixAWidth];
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < matrixAWidth; k++) {
				void* data = (uint8_t*)this->buf +
				             (matrixAAddr - this->buf_base_addr + (i * this->sa_size + j) * matrixAStride + 4 * k);
				matrixA[j * matrixAWidth + k] = *(uint32_t*)data;
			}
		}
		this->matrixAVector.push_back({matrixA, height, matrixAWidth});
	}
	for (int i = 0; i < sliceBCount; i++) {
		auto      width   = std::min(this->sa_size, matrixBWidth - i * this->sa_size);
		uint32_t* matrixB = new uint32_t[matrixBHeight * width];
		for (int j = 0; j < matrixBHeight; j++) {
			for (int k = 0; k < width; k++) {
				void* data = (uint8_t*)this->buf +
				             (matrixBAddr - this->buf_base_addr + j * matrixBStride + (i * this->sa_size + k) * 4);
				matrixB[j * width + k] = *(uint32_t*)data;
			}
		}
		this->matrixBVector.push_back({matrixB, matrixBHeight, width});
	}

	for (int i = 0; i < this->matrixAVector.size(); i++) {
		for (int j = 0; j < this->matrixBVector.size(); j++) {
			ASSERT_MSG(this->matrixAVector[i].width == this->matrixBVector[j].height,
			           "Dimension mismatch: A.width must equal B.height");
			auto addr = matrixCAddr + (i * this->sa_size * this->matrixCStride + 4 * j * this->sa_size);
			this->saCalQueue.push({this->matrixAVector[i].matrix, this->matrixBVector[j].matrix,
			                       this->matrixAVector[i].width, this->matrixAVector[i].height,
			                       this->matrixBVector[j].width, addr});
		}
	}

	SAInitEvent* saInitEvent = acalsim::top->getRecycleContainer()->acquire<SAInitEvent>(&SAInitEvent::renew, this);
	this->scheduleEvent(saInitEvent, acalsim::top->getGlobalTick() + 1);
}

void SystolicArray::initSystolicArray() {
	if (!this->saCalQueue.empty()) {
		auto calInfo = this->saCalQueue.front();
		this->saCalQueue.pop();

		this->matrixA        = calInfo.matrixA;
		this->matrixB        = calInfo.matrixB;
		this->matrixAWidth   = calInfo.matrixAWidth;
		this->matrixCHeight  = calInfo.matrixCHeight;
		this->matrixCWidth   = calInfo.matrixCWidth;
		this->matrixCAddr    = calInfo.matrixCAddr;
		this->estimatedCycle = calInfo.matrixCHeight + calInfo.matrixAWidth + calInfo.matrixCWidth - 2;
		this->currentCycle   = 0;

		for (int i = 0; i < this->matrixCHeight; i++) {
			for (int j = 0; j < this->matrixCWidth; j++) {
				int pos                                = i * this->sa_size + j;
				this->peArray[pos].a_in                = 0;
				this->peArray[pos].b_in                = 0;
				this->peArray[pos].pass_a_right        = false;
				this->peArray[pos].pass_b_down         = false;
				this->peArray[pos].has_input_from_top  = false;
				this->peArray[pos].has_input_from_left = false;
				this->peArray[pos].partial_sum         = 0;
			}
		}

		SAStepEvent* saStepEvent = acalsim::top->getRecycleContainer()->acquire<SAStepEvent>(&SAStepEvent::renew, this);
		this->scheduleEvent(saStepEvent, acalsim::top->getGlobalTick() + 1);
	} else {
		CLASS_ERROR << "Calculation done, should not trigger init!";
	}
}

void SystolicArray::systolicArrayStep() {
	// clear has_input_from_left, has_input_from_top
	for (int i = 0; i < this->matrixCHeight; i++) {
		for (int j = 0; j < this->matrixCWidth; j++) {
			int pos                                = i * this->sa_size + j;
			this->peArray[pos].has_input_from_left = false;
			this->peArray[pos].has_input_from_top  = false;
		}
	}
	// propogate from left to right
	for (int i = 0; i < this->matrixCHeight; i++) {
		for (int j = this->matrixCWidth - 2; j >= 0; j--) {
			int pos = i * this->sa_size + j;
			if (this->peArray[pos].pass_a_right) {
				this->peArray[pos + 1].a_in                = this->peArray[pos].a_in;
				this->peArray[pos + 1].has_input_from_left = true;
			}
			this->peArray[pos + 1].pass_a_right = this->peArray[pos].pass_a_right;
		}
	}
	// propogate from top to bottom
	for (int i = this->matrixCHeight - 2; i >= 0; i--) {
		for (int j = 0; j < this->matrixCWidth; j++) {
			int pos = i * this->sa_size + j;
			if (this->peArray[pos].pass_b_down) {
				this->peArray[pos + this->sa_size].b_in               = this->peArray[pos].b_in;
				this->peArray[pos + this->sa_size].has_input_from_top = true;
			}
			this->peArray[pos + this->sa_size].pass_b_down = this->peArray[pos].pass_b_down;
		}
	}
	// input data from A
	for (int i = 0; i < this->matrixCHeight; i++) {
		this->peArray[i * this->sa_size].pass_a_right = false;

		int a_col = this->currentCycle - i;
		if (a_col >= 0 && a_col < this->matrixAWidth) {
			this->peArray[i * this->sa_size].a_in                = this->matrixA[i * this->matrixAWidth + a_col];
			this->peArray[i * this->sa_size].has_input_from_left = true;
			this->peArray[i * this->sa_size].pass_a_right        = true;
		}
	}
	// input data from B
	for (int j = 0; j < this->matrixCWidth; j++) {
		this->peArray[j].pass_b_down = false;

		int b_row = this->currentCycle - j;
		if (b_row >= 0 && b_row < this->matrixAWidth) {
			this->peArray[j].b_in               = this->matrixB[b_row * this->matrixCWidth + j];
			this->peArray[j].has_input_from_top = true;
			this->peArray[j].pass_b_down        = true;
		}
	}
	// calculate partial sum
	for (int i = 0; i < this->matrixCHeight; i++) {
		for (int j = 0; j < this->matrixCWidth; j++) {
			int pos = i * this->sa_size + j;
			ASSERT_MSG(this->peArray[pos].has_input_from_top == this->peArray[pos].has_input_from_left,
			           "Error occured while propogating data");
			if (this->peArray[pos].has_input_from_top) {
				this->peArray[pos].partial_sum += this->peArray[pos].a_in * this->peArray[pos].b_in;
			}
		}
	}
	// schedule next step or calculation done
	this->currentCycle++;
	auto rc = acalsim::top->getRecycleContainer();
	if (this->currentCycle == this->estimatedCycle) {
		SACalDoneEvent* saCalDoneEvent = rc->acquire<SACalDoneEvent>(&SACalDoneEvent::renew, this);
		this->scheduleEvent(saCalDoneEvent, acalsim::top->getGlobalTick() + 1);
	} else {
		SAStepEvent* saStepEvent = rc->acquire<SAStepEvent>(&SAStepEvent::renew, this);
		this->scheduleEvent(saStepEvent, acalsim::top->getGlobalTick() + 1);
	}
}

void SystolicArray::systolicArrayCalDone() {
	for (int i = 0; i < this->matrixCHeight; i++) {
		for (int j = 0; j < this->matrixCWidth; j++) {
			int      pos   = i * this->sa_size + j;
			uint32_t val32 = this->peArray[pos].partial_sum;
			CLASS_INFO << "matrixC[" << i << "][" << j << "] = " << val32;
			std::memcpy(
			    (uint8_t*)this->buf + (this->matrixCAddr - this->buf_base_addr + i * this->matrixCStride + 4 * j),
			    &val32, sizeof(uint32_t));
		}
	}

	if (!this->saCalQueue.empty()) {
		SAInitEvent* saInitEvent = acalsim::top->getRecycleContainer()->acquire<SAInitEvent>(&SAInitEvent::renew, this);
		this->scheduleEvent(saInitEvent, acalsim::top->getGlobalTick() + 1);
	} else {
		SATriggerDoneEvent* saTriggerDoneEvent =
		    acalsim::top->getRecycleContainer()->acquire<SATriggerDoneEvent>(&SATriggerDoneEvent::renew, this);
		this->scheduleEvent(saTriggerDoneEvent, acalsim::top->getGlobalTick() + 1);
	}
}

void SystolicArray::saTriggerDone() {
	this->rf[0] &= ~1;
	this->rf[1] |= 1;
	this->rf[8] = 0x10101;

	while (this->matrixAVector.size()) {
		auto matrixA = this->matrixAVector.back().matrix;
		this->matrixAVector.pop_back();
		delete[] matrixA;
	}
	while (this->matrixBVector.size()) {
		auto matrixB = this->matrixBVector.back().matrix;
		this->matrixBVector.pop_back();
		delete[] matrixB;
	}
}

void SystolicArray::sendReadResp(MemReadRespPacket* _memRespPkt) {
	if (!this->getPipeRegister("sa2bus-resp-m")->isStalled()) {
		if (!this->getPipeRegister("sa2bus-resp-m")->push(_memRespPkt)) {
			CLASS_ERROR << "Cant push read resp from systolic array to bus";
		}
	} else {
		CLASS_ERROR << "Systolic array read response is stalled";
	}
}
