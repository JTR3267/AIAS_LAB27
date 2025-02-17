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

#ifndef SRC_APP_SOC_INCLUDE_CPUDEFS_HH_
#define SRC_APP_SOC_INCLUDE_CPUDEFS_HH_

#include <stdint.h>
#include <stdlib.h>

#define MEM_BYTES   0x10000
#define TEXT_OFFSET 0
#define DATA_OFFSET 8192

#define MAX_LABEL_COUNT 128
#define MAX_LABEL_LEN   32
#define MAX_SRC_LEN     (1024 * 1024)

#include <string>

#include "ACALSim.hh"
#include "Register.hh"

typedef enum {
	UNIMPL = 0,

	// instruction added
	// MUL,
	//*****************

	ADD,
	ADDI,
	AND,
	ANDI,
	AUIPC,
	BEQ,
	BGE,
	BGEU,
	BLT,
	BLTU,
	BNE,
	JAL,
	JALR,
	LB,
	LBU,
	LH,
	LHU,
	LUI,
	LW,
	OR,
	ORI,
	SB,
	SH,
	SLL,
	SLLI,
	SLT,
	SLTI,
	SLTIU,
	SLTU,
	SRA,
	SRAI,
	SRL,
	SRLI,
	SUB,
	SW,
	XOR,
	XORI,
	HCF
} instr_type;

typedef struct {
	char* src;
	int   offset;
} source;

typedef enum {
	OPTYPE_NONE,  // more like "don't care"
	OPTYPE_REG,
	OPTYPE_IMM,
	OPTYPE_LABEL,
} operand_type;

typedef struct {
	operand_type type = OPTYPE_NONE;
	char         label[MAX_LABEL_LEN];
	int          reg;
	uint32_t     imm;

} operand;

typedef struct {
	instr_type op;
	operand    a1;
	operand    a2;
	operand    a3;
	char*      psrc       = NULL;
	int        orig_line  = -1;
	bool       breakpoint = false;
} instr;

typedef struct {
	char label[MAX_LABEL_LEN];
	int  loc = -1;
} label_loc;

typedef struct {
	uint32_t pc;
	instr    inst;
} if_stage_out;

typedef struct {
	uint32_t pc;
	instr    inst;
	uint32_t rs1_data;
	uint32_t rs2_data;
	uint32_t immediate;
} id_stage_out;

typedef struct {
	uint32_t pc;
	instr    inst;
	uint32_t alu_out;  // computation result or address for load/store
	uint32_t write_data;
} exe_stage_out;

typedef struct {
	uint32_t pc;  // for logging
	instr    inst;
	union {
		uint32_t pc_plus_4_to_rd;
		uint32_t alu_out;
		uint32_t load_data;
	} mem_val;
} mem_stage_out;

struct Request {
	enum class ReqType { READ, WRITE };
	enum class ReqSize { BYTE, HALF, WORD };
	uint32_t  addr;
	uint32_t* data;
	ReqType   type;
	ReqSize   size;
};

class PerfCounter {
public:
	// Add a new performance counter
	PerfCounter() {}
	PerfCounter(const std::string& _name) : name(_name) {}
	~PerfCounter() {}
	// API to plus 1
	void counterPlusOne() { this->counter++; }
	// API to plus more than 1
	void counterPlusN(int _n) { this->counter += _n; }
	// Get the counter value
	int getCounter() { return this->counter; }
	// Log the counter value in one line
	void printCounterInfo() { INFO << "[" << this->name << " ]    " << this->counter; }

private:
	std::string name;
	int         counter = 0;
};

class RegFile {
public:
	RegFile() {}
	RegFile(int _size = 32) : size(_size) {
		this->regs = new std::pair<bool, Register<uint32_t>*>[_size];
		for (int i = 0; i < _size; i++) {
			this->regs[i].first  = false;
			this->regs[i].second = new Register<uint32_t>(std::make_shared<uint32_t>(0));
		}
	}
	~RegFile() {
		for (int i = 0; i < this->size; i++) { delete this->regs[i].second; }
		delete[] this->regs;
	};

	void printRegfile() {
		std::ostringstream oss;
		oss << "Pring Register File:\n";
		for (int i = 0; i < this->size; i++) {
			oss << "x" << std::setw(2) << std::setfill('0') << std::dec << i << ":0x";
			auto val       = this->regs[i].second->get();
			auto reg_value = (val) ? *val : 0;
			oss << std::setw(8) << std::setfill('0') << std::hex << reg_value << " ";
			if ((i + 1) % 8 == 0) { oss << "\n"; }
		}
		oss << "\n";
		INFO << oss.str();
	}

	void updateRegisterFile() {
		for (int i = 0; i < this->size; i++) {
			if (this->regs[i].first) {
				this->regs[i].second->update();
				this->regs[i].first = false;
			}
		}
	}

	const uint32_t& readRegister(int index) {
		ASSERT(index >= 0 && index < this->size);
		return *(this->regs[index].second->get());
	}

	void writeRegister(int index, uint32_t value) {
		ASSERT(index >= 0 && index < this->size);
		if (index != 0) {
			this->regs[index].first = true;
			this->regs[index].second->set(std::make_shared<uint32_t>(value));
		}
	}

private:
	std::pair<bool, Register<uint32_t>*>* regs;
	int                                   size;
};

#endif
