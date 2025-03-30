#ifndef HW2_INCLUDE_EVENT_SAREADRESPEVENT_HH_
#define HW2_INCLUDE_EVENT_SAREADRESPEVENT_HH_

#include "ACALSim.hh"

class SystolicArray;
class MemReadRespPacket;

class SAReadRespEvent : public acalsim::SimEvent {
public:
	SAReadRespEvent() = default;
	SAReadRespEvent(SystolicArray* _callee, MemReadRespPacket* _memRespPkt);
	virtual ~SAReadRespEvent() = default;

	void renew(SystolicArray* _callee, MemReadRespPacket* _memRespPkt);
	void process() override;

private:
	SystolicArray*     callee;
	MemReadRespPacket* memRespPkt;
};

#endif
