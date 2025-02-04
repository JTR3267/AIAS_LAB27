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

#ifndef SRC_APP_SOC_INCLUDE_INSTMEMORY_HH_
#define SRC_APP_SOC_INCLUDE_INSTMEMORY_HH_

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ACALSim.hh"
#include "CPUDefs.hh"

class InstMemory {
public:
	InstMemory() {
		imem = (instr*)malloc(DATA_OFFSET * sizeof(instr) / 4);
		for (int i = 0; i < DATA_OFFSET / 4; i++) {
			imem[i].op      = UNIMPL;
			imem[i].a1.type = OPTYPE_NONE;
			imem[i].a2.type = OPTYPE_NONE;
			imem[i].a3.type = OPTYPE_NONE;
		}
		this->labels      = (label_loc*)malloc(MAX_LABEL_COUNT * sizeof(label_loc));
		this->src.offset  = 0;
		this->src.src     = (char*)malloc(sizeof(char) * MAX_SRC_LEN);
		this->memoff      = 0;
		this->label_count = 0;
	};
	~InstMemory(){};
	void         parse(const std::string& file_path, uint8_t* mem);
	void         normalize_labels();
	const instr& fetchInstr(int index) { return this->imem[index]; };

private:
	instr*     imem;
	label_loc* labels;
	int        label_count;
	int        memoff;
	source     src;

	uint32_t   label_addr(char* label, int orig_line);
	void       append_source(const char* op, const char* a1, const char* a2, const char* a3, instr* i);
	int        parse_reg(char* tok, int line, bool strict = true);
	uint32_t   parse_imm(char* tok, int bits, int line, bool strict = true);
	void       parse_mem(char* tok, int* reg, uint32_t* imm, int bits, int line);
	void       parse_assembler_directive(int line, char* ftok, uint8_t* mem);
	int        parse_instr(int line, char* ftok);
	instr_type parse_instr(char* tok);
	int        parse_pseudoinstructions(int line, char* ftok, int ioff, char* o1, char* o2, char* o3, char* o4);
	void       parse_data_element(int line, int size, uint8_t* mem);

	void     print_syntax_error(int line, const char* msg);
	bool     streq(char* s, const char* q);
	uint32_t signextend(uint32_t in, int bits);
};

#endif