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

class PerfCounter {
public:
	// Get the performance counter value by name (CPU API)

	// Add a new performance counter
	// API to plus 1
	// API to plus more than 1
	// Log the counter value in one line
private:
}

#endif
