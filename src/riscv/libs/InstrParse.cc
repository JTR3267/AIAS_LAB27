#include "InstrParse.hh"

int getDestReg(const instr& _inst) {
	auto type = _inst.op;
	int  rd;
	switch (type) {
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

		case ADDI:
		case SLTI:
		case SLTIU:
		case ANDI:
		case ORI:
		case XORI:
		case SLLI:
		case SRLI:
		case SRAI:

		case JAL:
		case JALR:
		case AUIPC:
		case LUI:

		case LB:
		case LBU:
		case LH:
		case LHU:
		case LW: rd = _inst.a1.reg; break;

		default: rd = 0; break;
	}
	return rd;
}

bool checkDataHazard(int _rd, const instr& _inst) {
	auto type = _inst.op;
	int  rs1;
	int  rs2;
	switch (type) {
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
			rs1 = _inst.a2.reg;
			rs2 = _inst.a3.reg;
			break;

		case ADDI:
		case SLTI:
		case SLTIU:
		case ANDI:
		case ORI:
		case XORI:
		case SLLI:
		case SRLI:
		case SRAI:

		case JALR:

		case LB:
		case LBU:
		case LH:
		case LHU:
		case LW:
			rs1 = _inst.a2.reg;
			rs2 = 0;
			break;

		case BEQ:
		case BGE:
		case BGEU:
		case BLT:
		case BLTU:
		case BNE:

		case SB:
		case SH:
		case SW:
			rs1 = _inst.a1.reg;
			rs2 = _inst.a2.reg;
			break;

		default:
			rs1 = 0;
			rs2 = 0;
			break;
	}
	return (_rd == rs1 || _rd == rs2) && (_rd != 0);
}
