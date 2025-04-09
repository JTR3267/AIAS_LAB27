#ifndef HW2_INCLUDE_SYSTOLICARRAY_HH_
#define HW2_INCLUDE_SYSTOLICARRAY_HH_

#include <queue>
#include <string>
#include <vector>

#include "ACALSim.hh"
#include "MemPacket.hh"

typedef struct PE {
	uint32_t a_in;
	uint32_t b_in;
	bool     pass_a_right;
	bool     pass_b_down;
	bool     has_input_from_left;
	bool     has_input_from_top;
	uint32_t partial_sum;
} PE;

typedef struct MatrixInfo {
	uint32_t* matrix;
	uint32_t  height;
	uint32_t  width;
} MatrixInfo;

typedef struct CalInfo {
	uint32_t* matrixA;
	uint32_t* matrixB;
	uint32_t  matrixAWidth;
	uint32_t  matrixCHeight;
	uint32_t  matrixCWidth;
	uint32_t  matrixCAddr;
} CalInfo;

typedef struct SAReqInfo {
	acalsim::SimPacket* memReqPkt;
	uint32_t            data;
} SAReqInfo;

class SystolicArray : public acalsim::CPPSimBase {
public:
	SystolicArray(std::string _name, u_int32_t _sa_size, int _reg_base_addr, int _reg_size, int _buf_base_addr,
	              int _buf_size);

	virtual ~SystolicArray() { delete[] this->peArray; }

	void step() final;

	void triggerNextReq();

	void saReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt);

	void saReadReqHandler(MemReadReqPacket* _memReadReqPkt);

	void saWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, uint32_t data);

	void saParseConfig();

	void initSystolicArray();

	void systolicArrayStep();

	void systolicArrayCalDone();

	void saTriggerDone();

	void sendReadResp(MemReadRespPacket* _memRespPkt);

private:
	u_int32_t               sa_size;
	int                     reg_base_addr;
	int                     reg_size;
	int                     buf_base_addr;
	int                     buf_size;
	uint32_t                rf[9];
	uint32_t                buf[262144];
	bool                    is_idle = true;
	std::queue<SAReqInfo>   pending_req_queue;
	MemWriteReqPacket*      unmatchWriteReq  = nullptr;
	MemWriteDataPacket*     unmatchWriteData = nullptr;
	std::vector<MatrixInfo> matrixAVector;
	std::vector<MatrixInfo> matrixBVector;
	std::queue<CalInfo>     saCalQueue;
	uint32_t*               matrixA;
	uint32_t*               matrixB;
	PE*                     peArray;
	uint32_t                matrixAWidth;
	uint32_t                matrixCHeight;
	uint32_t                matrixCWidth;
	uint32_t                currentCycle;
	uint32_t                estimatedCycle;
	uint32_t                matrixCAddr;
	uint32_t                matrixCStride;
};

#endif
