#include "ACALSim.hh"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

class InstMemory{

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
    ~InstMemory() {};
	void parse(const std::string& file_path, uint8_t* mem);
	void normalize_labels();
    const instr& fetchInstr(int index) { return this->imem[index]; };


private:
    instr*   imem;
	label_loc*   labels;
	int          label_count;
	int          memoff;
	source       src;

	uint32_t   label_addr(char* label, int orig_line);
	void       append_source(const char* op, const char* a1, const char* a2, const char* a3, instr* i);
	int        parse_reg(char* tok, int line, bool strict = true);
	uint32_t   parse_imm(char* tok, int bits, int line, bool strict = true);
	void       parse_mem(char* tok, int* reg, uint32_t* imm, int bits, int line);
	void       parse_assembler_directive(int line, char* ftok, uint8_t* mem);
	int        parse_instr(int line, char* ftok);
	instr_type parse_instr(char* tok);
	int parse_pseudoinstructions(int line, char* ftok, int ioff, char* o1, char* o2,
	                             char* o3, char* o4);
	void parse_data_element(int line, int size, uint8_t* mem);

	void     print_syntax_error(int line, const char* msg);
	bool     streq(char* s, const char* q);
	uint32_t signextend(uint32_t in, int bits);


};


