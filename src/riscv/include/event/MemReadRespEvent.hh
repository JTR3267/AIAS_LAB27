#ifndef HW2_INCLUDE_EVENT_MEMREADRESPEVENT_HH_
#define HW2_INCLUDE_EVENT_MEMREADRESPEVENT_HH_

#include "ACALSim.hh"

class DataMemory;
class MemReadRespPacket;

class MemReadRespEvent : public acalsim::SimEvent {
public:
	MemReadRespEvent() = default;
	MemReadRespEvent(DataMemory* _callee, MemReadRespPacket* _memRespPkt);
	virtual ~MemReadRespEvent() = default;

	void renew(DataMemory* _callee, MemReadRespPacket* _memRespPkt);
	void process() override;

private:
	DataMemory*        callee;
	MemReadRespPacket* memRespPkt;
};

#endif
