#ifndef SRC_RISCV_INCLUDE_INSTRPARSE_HH_
#define SRC_RISCV_INCLUDE_INSTRPARSE_HH_

#include "DataStruct.hh"

int  getDestReg(const instr& _inst);
bool checkDataHazard(int _rd, const instr& _inst);

#endif
