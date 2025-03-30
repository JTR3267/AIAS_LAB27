#ifndef HW2_INCLUDE_DMA_HH_
#define HW2_INCLUDE_DMA_HH_

#include <queue>
#include <string>

#include "ACALSim.hh"
#include "MemPacket.hh"

typedef struct DMAReqInfo {
	acalsim::SimPacket* memReqPkt;
	int                 bytes;
	uint32_t            destAddr;
} DMAReqInfo;

class DMA : public acalsim::CPPSimBase {
public:
	DMA(std::string _name, int _base_addr, int _size);

	virtual ~DMA() {}

	void step() final;

	void triggerNextReq();

	void dmaReqHandler(acalsim::Tick _when, acalsim::SimPacket* _memReqPkt);

	void dmaReadReqHandler(MemReadReqPacket* _memReadReqPkt);

	void dmaWriteReqHandler(MemWriteReqPacket* _memWriteReqPkt, uint32_t data);

	void dmaReadRespHandler(acalsim::Tick _when, MemReadRespPacket* _memReadRespPkt);

	void initiateDmaTransfer();

	void triggerDmaTransfer();

	void sendReadResp(MemReadRespPacket* _memRespPkt);

	void sendReadReq(MemReadReqPacket* _memReadReqPkt);

	void sendWriteReq(MemWriteReqPacket* _memWriteReqPkt);

	void sendWriteData(MemWriteDataPacket* _memWriteDataPkt);

private:
	int                    base_addr;
	int                    size;
	uint32_t               rf[6];
	bool                   is_idle = true;
	std::queue<DMAReqInfo> pending_req_queue;
	std::queue<DMAReqInfo> dma_req_queue;
	uint32_t               curDestAddr;
	int                    curBytes;
	MemWriteReqPacket*     unmatchWriteReq  = nullptr;
	MemWriteDataPacket*    unmatchWriteData = nullptr;
};

#endif
