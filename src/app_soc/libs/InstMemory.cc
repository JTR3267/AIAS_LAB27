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

#include "InstMemory.hh"

bool InstMemory::streq(char* s, const char* q) {
	if (strcmp(s, q) == 0) return true;

	return false;
}

uint32_t InstMemory::signextend(uint32_t in, int bits) {
	if (in & (1 << (bits - 1))) return ((-1) << bits) | in;
	return in;
}

void InstMemory::print_syntax_error(int line, const char* msg) {
	ERROR << "Line " << line << ": Syntax error! " << msg;
}

void InstMemory::append_source(const char* op, const char* a1, const char* a2, const char* a3, instr* i) {
	char tbuf[128];  // not safe... static size... but should be okay since label length enforced
	if (op && a1 && !a2 && !a3) {
		sprintf(tbuf, "%s %s", op, a1);
	} else if (op && a1 && a2 && !a3) {
		sprintf(tbuf, "%s %s, %s", op, a1, a2);
	} else if (op && a1 && a2 && a3) {
		sprintf(tbuf, "%s %s, %s, %s", op, a1, a2, a3);
	} else {
		return;
	}
	int slen = strlen(tbuf);
	if (slen + this->src.offset < MAX_SRC_LEN) {
		strncpy(this->src.src + this->src.offset, tbuf, strlen(tbuf));

		i->psrc = this->src.src + this->src.offset;
		this->src.offset += slen + 1;
	}
}

int InstMemory::parse_reg(char* tok, int line, bool strict) {
	if (tok[0] == 'x') {
		int ri = atoi(tok + 1);
		if (ri < 0 || ri > 32) {
			if (strict) print_syntax_error(line, "Malformed register name");
			return -1;
		}
		return ri;
	}
	if (streq(tok, "zero")) return 0;
	if (streq(tok, "ra")) return 1;
	if (streq(tok, "sp")) return 2;
	if (streq(tok, "gp")) return 3;
	if (streq(tok, "tp")) return 4;
	if (streq(tok, "t0")) return 5;
	if (streq(tok, "t1")) return 6;
	if (streq(tok, "t2")) return 7;
	if (streq(tok, "s0")) return 8;
	if (streq(tok, "s1")) return 9;
	if (streq(tok, "a0")) return 10;
	if (streq(tok, "a1")) return 11;
	if (streq(tok, "a2")) return 12;
	if (streq(tok, "a3")) return 13;
	if (streq(tok, "a4")) return 14;
	if (streq(tok, "a5")) return 15;
	if (streq(tok, "a6")) return 16;
	if (streq(tok, "a7")) return 17;
	if (streq(tok, "s2")) return 18;
	if (streq(tok, "s3")) return 19;
	if (streq(tok, "s4")) return 20;
	if (streq(tok, "s5")) return 21;
	if (streq(tok, "s6")) return 22;
	if (streq(tok, "s7")) return 23;
	if (streq(tok, "s8")) return 24;
	if (streq(tok, "s9")) return 25;
	if (streq(tok, "s10")) return 26;
	if (streq(tok, "s11")) return 27;
	if (streq(tok, "t3")) return 28;
	if (streq(tok, "t4")) return 29;
	if (streq(tok, "t5")) return 30;
	if (streq(tok, "t6")) return 31;

	if (strict) print_syntax_error(line, "Malformed register name");
	return -1;
}

uint32_t InstMemory::parse_imm(char* tok, int bits, int line, bool strict) {
	if (!(tok[0] >= '0' && tok[0] <= '9') && tok[0] != '-' && strict) {
		print_syntax_error(line, "Malformed immediate value");
	}
	long int imml = strtol(tok, NULL, 0);

	if (imml > ((1 << bits) - 1) || imml < -(1 << (bits - 1))) {
		printf("Syntax error at token %s\n", tok);
		exit(1);
	}
	uint64_t uv = *(uint64_t*)&imml;
	uint32_t hv = (uv & UINT32_MAX);

	return hv;
}

void InstMemory::parse_mem(char* tok, int* reg, uint32_t* imm, int bits, int line) {
	char* imms = strtok(tok, "(");
	char* regs = strtok(NULL, ")");
	*imm       = parse_imm(imms, bits, line);
	*reg       = parse_reg(regs, line);
}

void InstMemory::parse_data_element(int line, int size, uint8_t* mem) {
	while (char* t = strtok(NULL, " \t\r\n")) {
		errno      = 0;
		int64_t v  = strtol(t, NULL, 0);
		int64_t vs = (v >> (size * 8));
		if (errno == ERANGE || (vs > 0 && vs != -1)) {
			printf("Value out of bounds at line %d : %s\n", line, t);
			exit(2);
		}
		// printf ( "parse_data_element %d: %d %ld %d %d\n", line, size, v, errno, sizeof(long int));
		memcpy(&mem[this->memoff], &v, size);
		this->memoff += size;
		// strtok(NULL, ",");
	}
}

void InstMemory::parse_assembler_directive(int line, char* ftok, uint8_t* mem) {
	// printf( "assembler directive %s\n", ftok );
	if (0 == memcmp(ftok, ".text", strlen(ftok))) {
		if (strtok(NULL, " \t\r\n")) { print_syntax_error(line, "Tokens after assembler directive"); }
		// cur_section = SECTION_TEXT;
		this->memoff = TEXT_OFFSET;
		// printf( "starting text section\n" );
	} else if (0 == memcmp(ftok, ".data", strlen(ftok))) {
		// cur_section = SECTION_TEXT;
		this->memoff = DATA_OFFSET;
		// printf( "starting data section\n" );
	} else if (0 == memcmp(ftok, ".byte", strlen(ftok)))
		parse_data_element(line, 1, mem);
	else if (0 == memcmp(ftok, ".half", strlen(ftok)))
		parse_data_element(line, 2, mem);
	else if (0 == memcmp(ftok, ".word", strlen(ftok)))
		parse_data_element(line, 4, mem);
	else {
		printf("Undefined assembler directive at line %d: %s\n", line, ftok);
		exit(3);
	}
}

int InstMemory::parse_pseudoinstructions(int line, char* ftok, int ioff, char* o1, char* o2, char* o3, char* o4) {
	if (streq(ftok, "li")) {
		if (!o1 || !o2 || o3) print_syntax_error(line, "Invalid format");

		int      reg  = parse_reg(o1, line);
		long int imml = strtol(o2, NULL, 0);

		if (reg < 0 || imml > UINT32_MAX || imml < INT32_MIN) {
			printf("Syntax error at line %d -- %lx, %x\n", line, imml, INT32_MAX);
			exit(1);
		}
		uint64_t uv = *(uint64_t*)&imml;
		uint32_t hv = (uv & UINT32_MAX);

		char areg[4];
		sprintf(areg, "x%02d", reg);
		char immu[12];
		sprintf(immu, "0x%08x", (hv >> 12));
		char immd[12];
		sprintf(immd, "0x%08x", (hv & ((1 << 12) - 1)));

		instr* i     = &(this->imem[ioff]);
		i->op        = LUI;
		i->a1.type   = OPTYPE_REG;
		i->a1.reg    = reg;
		i->a2.type   = OPTYPE_IMM;
		i->a2.imm    = hv >> 12;
		i->orig_line = line;
		append_source("lui", areg, immu, NULL, i);
		instr* i2 = &(this->imem[ioff + 1]);

		i2->op        = ADDI;
		i2->a1.type   = OPTYPE_REG;
		i2->a1.reg    = reg;
		i2->a2.type   = OPTYPE_REG;
		i2->a2.reg    = reg;
		i2->a3.type   = OPTYPE_IMM;
		i2->a3.imm    = (hv & ((1 << 12) - 1));
		i2->orig_line = line;
		append_source("addi", areg, areg, immd, i2);
		// printf( ">> %d %x %d\n", reg, i->a2.imm, i->a2.imm );
		// printf( ">> %d %x %d\n", reg, i2->a3.imm, i2->a3.imm );
		return 2;
	}
	if (streq(ftok, "la")) {
		if (!o1 || !o2 || o3) print_syntax_error(line, "Invalid format");

		int reg = parse_reg(o1, line);

		instr* i   = &(this->imem[ioff]);
		i->op      = LUI;
		i->a1.type = OPTYPE_REG;
		i->a1.reg  = reg;
		i->a2.type = OPTYPE_LABEL;
		strncpy(i->a2.label, o2, MAX_LABEL_LEN);
		i->orig_line = line;
		// append_source(ftok, o1, o2, o3, src, i); // done in normalize
		instr* i2   = &(this->imem[ioff + 1]);
		i2->op      = ADDI;
		i2->a1.type = OPTYPE_REG;
		i2->a1.reg  = reg;
		i2->a2.type = OPTYPE_REG;
		i2->a2.reg  = reg;
		i2->a3.type = OPTYPE_LABEL;
		strncpy(i2->a3.label, o2, MAX_LABEL_LEN);
		i2->orig_line = line;
		// append_source(ftok, o1, o2, o3, src, i2); // done in normalize
		return 2;
	}
	if (streq(ftok, "ret")) {
		if (o1) print_syntax_error(line, "Invalid format");

		instr* i     = &(this->imem[ioff]);
		i->op        = JALR;
		i->a1.type   = OPTYPE_REG;
		i->a1.reg    = 0;
		i->a2.type   = OPTYPE_REG;
		i->a2.reg    = 1;
		i->a3.type   = OPTYPE_IMM;
		i->a3.imm    = 0;
		i->orig_line = line;
		append_source("jalr", "x0", "x1", "x0", i);
		return 1;
	}
	if (streq(ftok, "j")) {
		if (!o1 || o2) print_syntax_error(line, "Invalid format");

		instr* i   = &(this->imem[ioff]);
		i->op      = JAL;
		i->a1.type = OPTYPE_REG;
		i->a1.reg  = 0;
		i->a2.type = OPTYPE_LABEL;
		strncpy(i->a2.label, o1, MAX_LABEL_LEN);
		i->orig_line = line;
		append_source("j", "x0", o1, NULL, i);
		return 1;
	}
	if (streq(ftok, "mv")) {
		if (!o1 || !o2 || o3) print_syntax_error(line, "Invalid format");
		instr* i     = &(this->imem[ioff]);
		i->op        = ADDI;
		i->a1.type   = OPTYPE_REG;
		i->a1.reg    = parse_reg(o1, line);
		i->a2.type   = OPTYPE_REG;
		i->a2.reg    = parse_reg(o2, line);
		i->a3.type   = OPTYPE_IMM;
		i->a3.imm    = 0;
		i->orig_line = line;
		append_source("addi", o1, o2, NULL, i);
		return 1;
	}
	if (streq(ftok, "bnez")) {
		if (!o1 || !o2 || o3) print_syntax_error(line, "Invalid format");
		instr* i   = &(this->imem[ioff]);
		i->op      = BNE;
		i->a1.type = OPTYPE_REG;
		i->a1.reg  = parse_reg(o1, line);
		i->a2.type = OPTYPE_REG;
		i->a2.reg  = 0;
		i->a3.type = OPTYPE_LABEL;
		strncpy(i->a3.label, o2, MAX_LABEL_LEN);
		i->orig_line = line;
		append_source("bne", "x0", o1, o2, i);
		return 1;
	}
	if (streq(ftok, "beqz")) {
		if (!o1 || !o2 || o3) print_syntax_error(line, "Invalid format");
		instr* i   = &(this->imem[ioff]);
		i->op      = BEQ;
		i->a1.type = OPTYPE_REG;
		i->a1.reg  = parse_reg(o1, line);
		i->a2.type = OPTYPE_REG;
		i->a2.reg  = 0;
		i->a3.type = OPTYPE_LABEL;
		strncpy(i->a3.label, o2, MAX_LABEL_LEN);
		i->orig_line = line;
		append_source("beq", "x0", o1, o2, i);
		return 1;
	}
	return 0;
}

instr_type InstMemory::parse_instr(char* tok) {
	// instruction added
	// if ( streq(tok , "mul")) return MUL;
	//*****************

	if (streq(tok, "add")) return ADD;
	if (streq(tok, "sub")) return SUB;
	if (streq(tok, "slt")) return SLT;
	if (streq(tok, "sltu")) return SLTU;
	if (streq(tok, "and")) return AND;
	if (streq(tok, "or")) return OR;
	if (streq(tok, "xor")) return XOR;
	if (streq(tok, "sll")) return SLL;
	if (streq(tok, "srl")) return SRL;
	if (streq(tok, "sra")) return SRA;

	// 1r, imm -> 1r
	if (streq(tok, "addi")) return ADDI;
	if (streq(tok, "slti")) return SLTI;
	if (streq(tok, "sltiu")) return SLTIU;
	if (streq(tok, "andi")) return ANDI;
	if (streq(tok, "ori")) return ORI;
	if (streq(tok, "xori")) return XORI;
	if (streq(tok, "slli")) return SLLI;
	if (streq(tok, "srli")) return SRLI;
	if (streq(tok, "srai")) return SRAI;

	// load/store
	if (streq(tok, "lb")) return LB;
	if (streq(tok, "lbu")) return LBU;
	if (streq(tok, "lh")) return LH;
	if (streq(tok, "lhu")) return LHU;
	if (streq(tok, "lw")) return LW;
	if (streq(tok, "sb")) return SB;
	if (streq(tok, "sh")) return SH;
	if (streq(tok, "sw")) return SW;

	// branch
	if (streq(tok, "beq")) return BEQ;
	if (streq(tok, "bge")) return BGE;
	if (streq(tok, "bgeu")) return BGEU;
	if (streq(tok, "blt")) return BLT;
	if (streq(tok, "bltu")) return BLTU;
	if (streq(tok, "bne")) return BNE;

	// jal
	if (streq(tok, "jal")) return JAL;
	if (streq(tok, "jalr")) return JALR;

	// lui
	if (streq(tok, "auipc")) return AUIPC;
	if (streq(tok, "lui")) return LUI;

	// unimpl
	// if ( streq(tok, "unimpl") ) return UNIMPL;
	if (streq(tok, "hcf")) return HCF;
	return UNIMPL;
}

int InstMemory::parse_instr(int line, char* ftok) {
	if (this->memoff + 4 > DATA_OFFSET) {
		printf("Instructions in data segment!\n");
		exit(1);
	}
	char* o1 = strtok(NULL, " \t\r\n,");
	char* o2 = strtok(NULL, " \t\r\n,");
	char* o3 = strtok(NULL, " \t\r\n,");
	char* o4 = strtok(NULL, " \t\r\n,");

	int ioff  = this->memoff / 4;
	int pscnt = parse_pseudoinstructions(line, ftok, ioff, o1, o2, o3, o4);
	if (pscnt > 0) {
		return pscnt;
	} else {
		instr*     i  = &(this->imem[ioff]);
		instr_type op = parse_instr(ftok);
		i->op         = op;
		i->orig_line  = line;
		append_source(ftok, o1, o2, o3, i);

		switch (op) {
			case UNIMPL:
				return 1;

				// instruction added
				//  case MUL:
				//      if ( !o1 || !o2 || !o3 || o4 ) print_syntax_error( line,  "Invalid format" );
				//  	    i->a1.reg = parse_reg(o1 , line);
				//  	    i->a2.reg = parse_reg(o2 , line);
				//  	    i->a3.reg = parse_reg(o3 , line);
				//      return 1;
				//****************

			case JAL:
				if (o2) {  // two operands, reg, label
					if (!o1 || !o2 || o3 || o4) print_syntax_error(line, "Invalid format");
					i->a1.type = OPTYPE_REG;
					i->a1.reg  = parse_reg(o1, line);
					i->a2.type = OPTYPE_LABEL;
					strncpy(i->a2.label, o2, MAX_LABEL_LEN);
				} else {  // one operand, label
					if (!o1 || o2 || o3 || o4) print_syntax_error(line, "Invalid format");

					i->a1.type = OPTYPE_REG;
					i->a1.reg  = 1;
					i->a2.type = OPTYPE_LABEL;
					strncpy(i->a2.label, o1, MAX_LABEL_LEN);
				}
				return 1;
			case JALR:
				if (!o1 || !o2 || o3 || o4) print_syntax_error(line, "Invalid format");
				i->a1.reg = parse_reg(o1, line);
				parse_mem(o2, &i->a2.reg, &i->a3.imm, 12, line);
				return 1;
			case ADD:
			case SUB:
			case SLT:
			case SLTU:
			case AND:
			case OR:
			case XOR:
			case SLL:
			case SRL:
			case SRA:
				if (!o1 || !o2 || !o3 || o4) print_syntax_error(line, "Invalid format");
				i->a1.reg = parse_reg(o1, line);
				i->a2.reg = parse_reg(o2, line);
				i->a3.reg = parse_reg(o3, line);
				return 1;
			case LB:
			case LBU:
			case LH:
			case LHU:
			case LW:
			case SB:
			case SH:
			case SW:
				if (!o1 || !o2 || o3 || o4) print_syntax_error(line, "Invalid format");
				i->a1.reg = parse_reg(o1, line);
				parse_mem(o2, &i->a2.reg, &i->a3.imm, 12, line);
				return 1;
			case ADDI:
			case SLTI:
			case SLTIU:
			case ANDI:
			case ORI:
			case XORI:
			case SLLI:
			case SRLI:
			case SRAI:
				if (!o1 || !o2 || !o3 || o4) print_syntax_error(line, "Invalid format");

				i->a1.reg = parse_reg(o1, line);
				i->a2.reg = parse_reg(o2, line);
				i->a3.imm = signextend(parse_imm(o3, 12, line), 12);
				return 1;
			case BEQ:
			case BGE:
			case BGEU:
			case BLT:
			case BLTU:
			case BNE:
				if (!o1 || !o2 || !o3 || o4) print_syntax_error(line, "Invalid format");
				i->a1.reg  = parse_reg(o1, line);
				i->a2.reg  = parse_reg(o2, line);
				i->a3.type = OPTYPE_LABEL;
				strncpy(i->a3.label, o3, MAX_LABEL_LEN);
				return 1;
			case LUI:
			case AUIPC:  // how to deal with LSB correctly? FIXME
				if (!o1 || !o2 || o3 || o4) print_syntax_error(line, "Invalid format");
				i->a1.reg = parse_reg(o1, line);
				i->a2.imm = (parse_imm(o2, 20, line));
				return 1;
			case HCF: return 1;
		}
	}
	return 1;
}

void InstMemory::parse(const std::string& file_path, uint8_t* mem) {
	FILE* fin = fopen(file_path.c_str(), "r");
	if (!fin) { ERROR << file_path << ": No such file"; }
	int line = 0;

	INFO << "Parsing input file";

	// sectionType cur_section = SECTION_NONE;

	char rbuf[1024];
	while (!feof(fin)) {
		if (!fgets(rbuf, 1024, fin)) break;
		for (char* p = rbuf; *p; ++p) *p = tolower(*p);
		line++;

		char* ftok = strtok(rbuf, " \t\r\n");
		if (!ftok) continue;

		if (ftok[0] == '#') continue;
		if (ftok[0] == '.') {
			parse_assembler_directive(line, ftok, mem);
		} else if (ftok[strlen(ftok) - 1] == ':') {
			ftok[strlen(ftok) - 1] = 0;
			if (strlen(ftok) >= MAX_LABEL_LEN) {
				printf("Exceeded maximum length of label: %s\n", ftok);
				exit(3);
			}
			if (this->label_count >= MAX_LABEL_COUNT) { ERROR << "Exceeded maximum number of supported labels"; }
			strncpy(this->labels[this->label_count].label, ftok, MAX_LABEL_LEN);
			this->labels[this->label_count].loc = this->memoff;
			this->label_count++;
			// printf( "Parsing label %s at mem %x\n", ftok, memoff );

			char* ntok = strtok(NULL, " \t\r\n");
			// there is more code after label
			if (ntok) {
				if (ntok[0] == '.') {
					parse_assembler_directive(line, ntok, mem);
				} else {
					int count = parse_instr(line, ntok);
					for (int i = 0; i < count; i++) *(uint32_t*)&mem[this->memoff + (i * 4)] = 0xcccccccc;
					this->memoff += count * 4;
				}
			}
		} else {
			int count = parse_instr(line, ftok);
			for (int i = 0; i < count; i++) *(uint32_t*)&mem[this->memoff + (i * 4)] = 0xcccccccc;
			this->memoff += count * 4;
		}
	}
}

uint32_t InstMemory::label_addr(char* label, int orig_line) {
	for (int i = 0; i < this->label_count; i++) {
		if (streq(this->labels[i].label, label)) return this->labels[i].loc;
	}
	print_syntax_error(orig_line, "Undefined label");
	return 0;
}

void InstMemory::normalize_labels() {
	for (int i = 0; i < DATA_OFFSET / 4; i++) {
		instr* ii = &(this->imem[i]);
		if (ii->op == UNIMPL) continue;

		if (ii->a1.type == OPTYPE_LABEL) {
			ii->a1.type = OPTYPE_IMM;
			ii->a1.imm  = label_addr(ii->a1.label, ii->orig_line);
		}
		if (ii->a2.type == OPTYPE_LABEL) {
			ii->a2.type = OPTYPE_IMM;
			ii->a2.imm  = label_addr(ii->a2.label, ii->orig_line);
			switch (ii->op) {
				case LUI: {
					ii->a2.imm = (ii->a2.imm >> 12);
					char areg[4];
					sprintf(areg, "x%02d", ii->a1.reg);
					char immu[12];
					sprintf(immu, "0x%08x", ii->a2.imm);
					// printf( "LUI %d 0x%x %s\n", ii->a1.reg, ii->a2.imm, immu );
					append_source("lui", areg, immu, NULL, ii);
					break;
				}
				case JAL:
					int pc     = (i * 4);
					int target = ii->a3.imm;
					int diff   = pc - target;
					if (diff < 0) diff = -diff;

					if (diff >= (1 << 21)) {
						printf("JAL instruction target out of bounds\n");
						exit(3);
					}
					break;
			}
		}
		if (ii->a3.type == OPTYPE_LABEL) {
			ii->a3.type = OPTYPE_IMM;
			ii->a3.imm  = label_addr(ii->a3.label, ii->orig_line);
			switch (ii->op) {
				case ADDI: {
					ii->a3.imm = ii->a3.imm & ((1 << 12) - 1);
					char a1reg[4];
					sprintf(a1reg, "x%02d", ii->a1.reg);
					char a2reg[4];
					sprintf(a2reg, "x%02d", ii->a2.reg);
					char immd[12];
					sprintf(immd, "0x%08x", ii->a3.imm);
					// printf( "ADDI %d %d 0x%x %s\n", ii->a1.reg, ii->a2.reg, ii->a3.imm, immd );
					append_source("addi", a1reg, a2reg, immd, ii);
					break;
				}
				case BEQ:
				case BGE:
				case BGEU:
				case BLT:
				case BLTU:
				case BNE: {
					int pc     = (i * 4);
					int target = ii->a3.imm;
					int diff   = pc - target;
					if (diff < 0) diff = -diff;

					if (diff >= (1 << 13)) {
						printf("Branch instruction target out of bounds\n");
						exit(3);
					}
					break;
				}
			}
		}
	}
}
