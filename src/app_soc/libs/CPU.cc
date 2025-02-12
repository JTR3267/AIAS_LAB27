/*
 * Copyright 2023-2024 Playlab/ACAL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CPU.hh"

#include "CPUSingleIterationEvent.hh"
#include "EXEStage.hh"
#include "IDStage.hh"
#include "IFStage.hh"
#include "MEMStage.hh"
#include "Register.hh"
#include "WBStage.hh"

CPU::CPU(const std::string& _name, const std::string& _m_port, const std::string& _s_port)
    : acalsim::CPPSimBase(_name) {
	this->imem = new InstMemory();
	for (int i = 0; i < 32; i++) {
		this->regs[i].first  = false;
		this->regs[i].second = new Register<uint32_t>(std::make_shared<uint32_t>(0));
	}

	this->if_id_reg   = new Register<if_stage_out>();
	this->id_exe_reg  = new Register<id_stage_out>();
	this->exe_mem_reg = new Register<exe_stage_out>();
	this->mem_wb_reg  = new Register<mem_stage_out>();

	this->if_trace_data  = new ChromeTraceData({.is_started = false, .stage_name = "IFStage", .event_name = "INIT"});
	this->id_trace_data  = new ChromeTraceData({.is_started = false, .stage_name = "IDStage", .event_name = "INIT"});
	this->exe_trace_data = new ChromeTraceData({.is_started = false, .stage_name = "EXEStage", .event_name = "INIT"});
	this->mem_trace_data = new ChromeTraceData({.is_started = false, .stage_name = "MEMStage", .event_name = "INIT"});
	this->wb_trace_data  = new ChromeTraceData({.is_started = false, .stage_name = "WBStage", .event_name = "INIT"});

	// Generate and Register MasterPorts
	this->m_port_ = this->addMasterPort(_m_port);
	// Generate and Register SlavePorts
	this->s_port_ = this->addSlavePort(_s_port, 1);
}

CPU::~CPU() {}

void CPU::init() {
	CLASS_INFO << "CPU initialization and register SimModules";
	this->registerModules();
	for (auto& [_, module] : this->modules) { module->init(); }

	std::string asm_file_path = acalsim::top->getParameter<std::string>("SOC", "asm_file_path");

	DataMemory* dmem = dynamic_cast<DataMemory*>(this->getDownStream("DSDMEM"));
	this->imem->parse(asm_file_path, static_cast<uint8_t*>(dmem->getMemPtr()));
	this->imem->normalize_labels();

	CLASS_INFO << "Simulation starts";

	auto rc    = acalsim::top->getRecycleContainer();
	auto event = rc->acquire<CPUSingleIterationEvent>(&CPUSingleIterationEvent::renew, this);
	this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);

	IFStage* if_stage   = dynamic_cast<IFStage*>(this->getModule("IFStage"));
	uint32_t current_pc = if_stage->getCurPC();
	int      index      = current_pc / 4;
	this->recordTrace<uint32_t>(if_stage->getPCReg(), this->instrToString(this->fetchInstr(index).op),
	                            this->if_trace_data);
}

void CPU::registerModules() {
	// Generate and Register Modules
	auto IFStage_mod  = new IFStage("IFStage", this->if_id_reg);
	auto IDStage_mod  = new IDStage("IDStage", this->if_id_reg, this->id_exe_reg);
	auto EXEStage_mod = new EXEStage("EXEStage", this->id_exe_reg, this->exe_mem_reg);
	auto MEMStage_mod = new MEMStage("MEMStage", this->exe_mem_reg, this->mem_wb_reg);
	auto WBStage_mod  = new WBStage("WBStage", this->mem_wb_reg);

	this->addModule(IFStage_mod);
	this->addModule(IDStage_mod);
	this->addModule(EXEStage_mod);
	this->addModule(MEMStage_mod);
	this->addModule(WBStage_mod);

	// Connect SimPort
}

template <typename T>
void CPU::recordTrace(Register<T>* reg, std::string inst_name, ChromeTraceData* data) {
	auto        info            = reg->get();
	std::string inst_event_name = inst_name + " (pc = " + std::to_string(*info + 4) + ")" + " - " + data->stage_name;
	if (data->is_started) {
		acalsim::top->addChromeTraceRecord(acalsim::ChromeTraceRecord::createDurationEvent(
		    "E", "CPU", data->event_name, acalsim::top->getGlobalTick(), "", data->stage_name));
		if (inst_name != "UNIMPL") {
			acalsim::top->addChromeTraceRecord(acalsim::ChromeTraceRecord::createDurationEvent(
			    "B", "CPU", inst_event_name, acalsim::top->getGlobalTick(), "", data->stage_name));
		}
	} else {
		if (inst_name != "UNIMPL") {
			acalsim::top->addChromeTraceRecord(acalsim::ChromeTraceRecord::createDurationEvent(
			    "B", "CPU", inst_event_name, acalsim::top->getGlobalTick(), "", data->stage_name));
		}
	}

	data->event_name = inst_event_name;
	data->is_started = true;
}

template <typename T>
void CPU::recordTrace(Register<T>* reg, ChromeTraceData* data) {
	auto info = reg->get();

	std::string inst_name = info == nullptr
	                            ? "NOP (tick = " + std::to_string(acalsim::top->getGlobalTick()) + ")"
	                            : this->instrToString(info->inst.op) + " (pc = " + std::to_string(info->pc) + ")";
	inst_name             = inst_name + " - " + data->stage_name;
	if (data->is_started) {
		acalsim::top->addChromeTraceRecord(acalsim::ChromeTraceRecord::createDurationEvent(
		    "E", "CPU", data->event_name, acalsim::top->getGlobalTick(), "", data->stage_name));
	}
	if (!info || info->inst.op != UNIMPL) {
		acalsim::top->addChromeTraceRecord(acalsim::ChromeTraceRecord::createDurationEvent(
		    "B", "CPU", inst_name, acalsim::top->getGlobalTick(), "", data->stage_name));
	}

	data->event_name = inst_name;
	data->is_started = true;
}

void CPU::execDataPath() {
	dynamic_cast<WBStage*>(this->getModule("WBStage"))->execDataPath();
	dynamic_cast<MEMStage*>(this->getModule("MEMStage"))->execDataPath();
	dynamic_cast<EXEStage*>(this->getModule("EXEStage"))->execDataPath();
	dynamic_cast<IDStage*>(this->getModule("IDStage"))->execDataPath();
	dynamic_cast<IFStage*>(this->getModule("IFStage"))->execDataPath();
}

void CPU::updatePipeRegisters() {
	this->if_id_reg->update([this]() { this->recordTrace<if_stage_out>(this->if_id_reg, this->id_trace_data); });
	this->id_exe_reg->update([this]() { this->recordTrace<id_stage_out>(this->id_exe_reg, this->exe_trace_data); });
	this->exe_mem_reg->update([this]() { this->recordTrace<exe_stage_out>(this->exe_mem_reg, this->mem_trace_data); });
	this->mem_wb_reg->update([this]() { this->recordTrace<mem_stage_out>(this->mem_wb_reg, this->wb_trace_data); });
}

void CPU::updateRegisterFile() {
	for (int i = 0; i < 32; i++) {
		if (this->regs[i].first) {
			this->regs[i].second->update();
			this->regs[i].first = false;
		}
	}
}

void CPU::printRegfile() {
	std::ostringstream oss;

	oss << "CPU Simulation is done\n";

	for (int i = 0; i < 32; i++) {
		oss << "x" << std::setw(2) << std::setfill('0') << std::dec << i << ":0x";

		auto val = regs[i].second->get();

		uint32_t reg_value = (val) ? *val : 0;

		oss << std::setw(8) << std::setfill('0') << std::hex << reg_value << " ";

		if ((i + 1) % 8 == 0) { oss << "\n"; }
	}

	oss << "\n";

	CLASS_INFO << oss.str();
}

void CPU::updateStatus() {
	dynamic_cast<IFStage*>(this->getModule("IFStage"))->updateStatus();
	dynamic_cast<IDStage*>(this->getModule("IDStage"))->updateStatus();
	dynamic_cast<EXEStage*>(this->getModule("EXEStage"))->updateStatus();
}

void CPU::updatePC() { dynamic_cast<IFStage*>(this->getModule("IFStage"))->updatePC(); }

void CPU::checkNextCycleEvent() {
	bool stall = dynamic_cast<IFStage*>(this->getModule("IFStage"))->getStallStatus() &&
	             dynamic_cast<IDStage*>(this->getModule("IDStage"))->getStallStatus() &&
	             dynamic_cast<EXEStage*>(this->getModule("EXEStage"))->getStallStatus();
	bool hcf = dynamic_cast<WBStage*>(this->getModule("WBStage"))->checkHcf();
	if (!stall && !hcf) {
		auto rc    = acalsim::top->getRecycleContainer();
		auto event = rc->acquire<CPUSingleIterationEvent>(&CPUSingleIterationEvent::renew, this);
		this->scheduleEvent(event, acalsim::top->getGlobalTick() + 1);
	} else if (hcf) {
		CLASS_INFO << "HCF instruction detected in WB stage. Simulation ends.";
		this->printRegfile();
	}
}

int CPU::getDestReg(const instr& _inst) {
	auto type = _inst.op;
	int  rd;
	switch (type) {
		case ADD:
		case ADDI:
		case LW:
		case LUI:
		case JAL: rd = _inst.a1.reg; break;
		case BEQ:
		case SB:
		default: rd = 0; break;
	}
	return rd;
}

bool CPU::checkDataHazard(int _rd, std::string _stage) {
	// Get rs1 and rs2 from the ID stage inbound register
	auto id_reg = dynamic_cast<IDStage*>(this->getModule("IDStage"))->getRegInfoFromID();
	if (!id_reg) return false;
	auto type = id_reg->inst.op;
	int  rs1;
	int  rs2;
	switch (type) {
		case ADD:
			rs1 = id_reg->inst.a2.reg;
			rs2 = id_reg->inst.a3.reg;
			break;
		case ADDI:
			rs1 = id_reg->inst.a2.reg;
			rs2 = 0;
			break;
		case BEQ:
			rs1 = id_reg->inst.a1.reg;
			rs2 = id_reg->inst.a2.reg;
			break;
		case SB:
			rs1 = id_reg->inst.a1.reg;
			rs2 = id_reg->inst.a2.reg;
			break;
		case LW:
			rs1 = id_reg->inst.a2.reg;
			rs2 = 0;
			break;
		case LUI:
		case JAL:
		default:
			rs1 = 0;
			rs2 = 0;
			break;
	}
	if (id_reg) {
		CLASS_INFO << "Detect IDStage rd = " << _rd << ", " << _stage << " rs1 = " << rs1 << " rs2 = " << rs2;
		return (_rd == rs1 || _rd == rs2) && (_rd != 0);
	}
	return false;
}

void CPU::updateSystemStates() {
	this->checkNextCycleEvent();
	this->updateStatus();
	this->updatePipeRegisters();
	this->updateRegisterFile();
	this->updatePC();
}

void CPU::step() {
	if (this->s_port_->isPopValid()) {
		auto packet = this->s_port_->pop();
		this->accept(acalsim::top->getGlobalTick(), *packet);
	}
}

void CPU::handler(MemRespPacket* _pkt) {
	// Update the state of MEM stage about the received packet
	dynamic_cast<MEMStage*>(this->getModule("MEMStage"))->setRespPkt(_pkt);

	CLASS_INFO << "----------- MemRespPacket received at CPU -----------";
	// Run the internal logic of all stages
	this->execDataPath();
	this->updateSystemStates();
	CLASS_INFO << "-----------------------";
}

std::string CPU::instrToString(instr_type op) {
	switch (op) {
		case UNIMPL: return "UNIMPL";

		// R-type
		case ADD: return "ADD";
		case AND: return "AND";
		case OR: return "OR";
		case XOR: return "XOR";
		case SUB: return "SUB";
		case SLL: return "SLL";
		case SRL: return "SRL";
		case SRA: return "SRA";
		case SLT: return "SLT";
		case SLTU: return "SLTU";

		// I-type
		case ADDI: return "ADDI";
		case ANDI: return "ANDI";
		case ORI: return "ORI";
		case XORI: return "XORI";
		case SLLI: return "SLLI";
		case SRLI: return "SRLI";
		case SRAI: return "SRAI";
		case SLTI: return "SLTI";
		case SLTIU: return "SLTIU";

		// Load
		case LB: return "LB";
		case LBU: return "LBU";
		case LH: return "LH";
		case LHU: return "LHU";
		case LW: return "LW";

		// Store
		case SB: return "SB";
		case SH: return "SH";
		case SW: return "SW";

		// Branch
		case BEQ: return "BEQ";
		case BNE: return "BNE";
		case BGE: return "BGE";
		case BGEU: return "BGEU";
		case BLT: return "BLT";
		case BLTU: return "BLTU";

		// Jump
		case JAL: return "JAL";
		case JALR: return "JALR";

		// Upper / Immediate
		case AUIPC: return "AUIPC";
		case LUI: return "LUI";

		// Special
		case HCF: return "HCF";

		default: return "UNKNOWN";
	}
}

void CPU::cleanup() {}

template void CPU::recordTrace<uint32_t>(Register<uint32_t>* reg, std::string inst_name, ChromeTraceData* data);