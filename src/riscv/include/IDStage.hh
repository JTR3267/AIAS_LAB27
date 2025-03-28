#ifndef SRC_RISCV_INCLUDE_IDSTAGE_HH_
#define SRC_RISCV_INCLUDE_IDSTAGE_HH_

#include <string>

#include "ACALSim.hh"
#include "Emulator.hh"
#include "InstPacket.hh"

class IDStage : public acalsim::CPPSimBase {
public:
	IDStage(std::string name) : acalsim::CPPSimBase(name) {}
	~IDStage() {}

	void init() override {}
	void step() override;
	void cleanup() override {}
	void instPacketHandler(Tick when, SimPacket* pkt);

private:
	bool        branchStall   = false;
	InstPacket* EXEInstPacket = nullptr;
	InstPacket* MEMInstPacket = nullptr;
	InstPacket* WBInstPacket  = nullptr;
};

#endif  // SRC_RISCV_INCLUDE_IDSTAGE_HH_
