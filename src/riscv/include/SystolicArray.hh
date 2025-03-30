#ifndef HW2_INCLUDE_SYSTOLICARRAY_HH_
#define HW2_INCLUDE_SYSTOLICARRAY_HH_

#include <queue>
#include <string>

#include "ACALSim.hh"
#include "MemPacket.hh"

typedef struct PE {
	uint32_t a_in;
	uint32_t b_in;
	bool     pass_a_right;
	bool     pass_b_down;
	bool     has_input_from_top;
	bool     has_input_from_left;
	uint32_t partial_sum;
} PE;

typedef struct SAReqInfo {
	acalsim::SimPacket* memReqPkt;
	uint32_t            data;
} SAReqInfo;

class SystolicArray : public acalsim::CPPSimBase {
public:
	SystolicArray(std::string _name, int _reg_base_addr, int _reg_size, int _buf_base_addr, int _buf_size);

	virtual ~SystolicArray() {}

	void step() final;

	void triggerNextReq();

	void saReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt);

	void saReadReqHandler(MemReadReqPacket* _memReadReqPkt);

	void saWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, uint32_t data);

	void initSystolicArray();

	void systolicArrayStep();

	void systolicArrayCalDone();

	void sendReadResp(MemReadRespPacket* _memRespPkt);

private:
	int                   reg_base_addr;
	int                   reg_size;
	int                   buf_base_addr;
	int                   buf_size;
	uint32_t              rf[9];
	uint32_t              buf[262144];
	bool                  is_idle = true;
	std::queue<SAReqInfo> pending_req_queue;
	MemWriteReqPacket*    unmatchWriteReq  = nullptr;
	MemWriteDataPacket*   unmatchWriteData = nullptr;
	uint32_t*             matrixA          = nullptr;
	uint32_t*             matrixB          = nullptr;
	PE*                   peArray          = nullptr;
	uint32_t              matrixAWidth;
	uint32_t              matrixCHeight;
	uint32_t              matrixCWidth;
	uint32_t              matrixCAddr;
	uint32_t              matrixCStride;
	uint32_t              currentCycle;
};

#endif
