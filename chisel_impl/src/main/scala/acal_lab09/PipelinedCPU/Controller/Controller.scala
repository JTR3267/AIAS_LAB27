package acal_lab09.PiplinedCPU.Controller

import chisel3._
import chisel3.util._

import acal_lab09.PiplinedCPU.opcode_map._
import acal_lab09.PiplinedCPU.condition._
import acal_lab09.PiplinedCPU.inst_type._
import acal_lab09.PiplinedCPU.alu_op_map._
import acal_lab09.PiplinedCPU.pc_sel_map._
import acal_lab09.PiplinedCPU.wb_sel_map._
import acal_lab09.PiplinedCPU.forwarding_sel_map._

class Controller(memAddrWidth: Int) extends Module {
  val io = IO(new Bundle {
    // Memory control signal interface
    val IM_Mem_R = Output(Bool())
    val IM_Mem_W = Output(Bool())
    val IM_Length = Output(UInt(4.W))
    val IM_Valid = Input(Bool())

    val DM_Mem_R = Output(Bool())
    val DM_Mem_W = Output(Bool())
    val DM_Length = Output(UInt(4.W))
    val DM_Valid = Input(Bool())

    // branch Comp.
    val E_BrEq = Input(Bool())
    val E_BrLT = Input(Bool())

    // Branch Prediction
    val E_En = Output(Bool())
    val E_Branch_taken = Output(Bool())

    val ID_pc = Input(UInt(memAddrWidth.W))
    val EXE_target_pc = Input(UInt(memAddrWidth.W))

    // Flush)
    val Flush_BH  = Output(Bool()) // branch hazard flush

    // Stall
    // To Be Modified
    val Stall_EXE_ID_DH = Output(Bool()) //TBD    // !
    val Stall_MEM_ID_DH = Output(Bool()) //TBD    // !
    val Stall_WB_ID_DH = Output(Bool()) //TBD
    val Stall_MA = Output(Bool()) //TBD

    // inst
    val IF_Inst = Input(UInt(32.W))
    val ID_Inst = Input(UInt(32.W))
    val EXE_Inst = Input(UInt(32.W))
    val MEM_Inst = Input(UInt(32.W))
    val WB_Inst = Input(UInt(32.W))

    // sel
    val PCSel = Output(UInt(2.W))
    val D_ImmSel = Output(UInt(3.W))
    val W_RegWEn = Output(Bool())
    val E_BrUn = Output(Bool())
    val E_ASel = Output(UInt(2.W))
    val E_BSel = Output(UInt(1.W))
    val E_ALUSel = Output(UInt(15.W))
    val W_WBSel = Output(UInt(2.W))

    val Hcf = Output(Bool())
  })
  // Inst Decode for each stage
  val IF_opcode = io.IF_Inst(6, 0)

  val ID_opcode = io.ID_Inst(6, 0)
  val ID_rs1 = io.ID_Inst(19,15)
  val ID_rs2 = io.ID_Inst(24,20)

  val EXE_opcode = io.EXE_Inst(6, 0)
  val EXE_funct3 = io.EXE_Inst(14, 12)
  val EXE_funct7 = io.EXE_Inst(31, 25)
  val EXE_rd = io.EXE_Inst(11, 7)

  val MEM_opcode = io.MEM_Inst(6, 0)
  val MEM_funct3 = io.MEM_Inst(14, 12)
  val MEM_rd = io.MEM_Inst(11, 7)

  val WB_opcode = io.WB_Inst(6, 0)
  val WB_rd = io.WB_Inst(11, 7)


  // Control signal - Branch/Jump
  val E_En = Wire(Bool())
  E_En := (EXE_opcode===BRANCH) ||
          (EXE_opcode===JALR)   ||
          (EXE_opcode===JAL)              // To Be Modified
  val E_Branch_taken = Wire(Bool())
  E_Branch_taken := MuxLookup(EXE_opcode, false.B, Seq(
          BRANCH -> MuxLookup(EXE_funct3, false.B, Seq(
            "b000".U(3.W) ->  io.E_BrEq.asUInt,  // beq
            "b001".U(3.W) -> ~io.E_BrEq.asUInt,  // bne
            "b100".U(3.W) ->  io.E_BrLT.asUInt,  // blt
            "b101".U(3.W) -> ~io.E_BrLT.asUInt,  // bge
            "b110".U(3.W) ->  io.E_BrLT.asUInt,  // bltu
            "b111".U(3.W) -> ~io.E_BrLT.asUInt,  // bgeu

            // TA's code
            // "b000".U(3.W) ->  io.E_BrEq.asUInt,
          )),
          JALR -> true.B,                 // !
          JAL  -> true.B                  // !
        ))    // To Be Modified

  io.E_En := E_En
  io.E_Branch_taken := E_Branch_taken

  // pc predict miss signal
  val Predict_Miss = Wire(Bool())
  Predict_Miss := (E_En && E_Branch_taken)

  // Control signal - PC
  when(Predict_Miss){
    io.PCSel := EXE_T_PC
  }.otherwise{
    io.PCSel := IF_PC_PLUS_4
  }

  // Control signal - Branch comparator
  io.E_BrUn := (io.EXE_Inst(13) === 1.U)

  // Control signal - Immediate generator
  io.D_ImmSel := MuxLookup(ID_opcode, 0.U, Seq(
    LOAD   -> I_type,
    STORE  -> S_type,
    BRANCH -> B_type,
    JALR   -> I_type,
    JAL    -> J_type,
    OP_IMM -> I_type,
    OP     -> R_type,   // no imm
    AUIPC  -> U_type,
    LUI    -> U_type,
    HCF    -> R_type,   // no imm

    // TA's code
    // OP_IMM -> I_type,
    // LOAD -> I_type,
    // BRANCH -> B_type,
    // LUI -> U_type,
  )) // To Be Modified

  // Control signal - Scalar ALU
  io.E_ASel := MuxLookup(EXE_opcode, 0.U, Seq(
    LOAD   -> 0.U,    // rs1
    STORE  -> 0.U,    // rs1
    BRANCH -> 1.U,    // PC
    JALR   -> 0.U,    // rs1
    JAL    -> 1.U,    // PC
    OP_IMM -> 0.U,    // rs1
    OP     -> 0.U,    // rs1
    AUIPC  -> 1.U,    // PC
    LUI    -> 2.U,    // 32'b0
    HCF    -> 2.U,    // dont care

    // TA's code
    // BRANCH -> 1.U,
    // LUI -> 2.U,
  ))    // To Be Modified

  io.E_BSel := MuxLookup(EXE_opcode, 0.U, Seq(
    LOAD   -> 1.U,    // imm
    STORE  -> 1.U,    // imm
    BRANCH -> 1.U,    // imm
    JALR   -> 1.U,    // imm
    JAL    -> 1.U,    // imm
    OP_IMM -> 1.U,    // imm
    OP     -> 0.U,    // rs2
    AUIPC  -> 1.U,    // imm
    LUI    -> 1.U,    // imm
    HCF    -> 0.U,    // dont care
  ))
  // TA's code
  // io.E_BSel := 1.U // To Be Modified

  io.E_ALUSel := MuxLookup(EXE_opcode, ADD, Seq(
    LOAD   -> ADD,    // rs1 + imm
    STORE  -> ADD,    // rs1 + imm
    BRANCH -> ADD,    // PC  + imm
    JALR   -> ADD,    // rs1 + imm
    JAL    -> ADD,    // PC  + imm
    OP_IMM -> Mux(EXE_funct3 === 5.U, 
              (Cat(EXE_funct7, "b11111".U, EXE_funct3)),    // srli or srai
              (Cat(0.U(7.W),   "b11111".U, EXE_funct3))),   // others
    OP     -> (Cat(EXE_funct7, "b11111".U, EXE_funct3)),
    AUIPC  -> ADD,    // PC  + imm
    LUI    -> ADD,    // 32'd0 + imm
    HCF    -> ADD,    // dont care

    // TA's code
    // OP -> (Cat(EXE_funct7, "b11111".U, EXE_funct3)),
    // OP_IMM -> (Cat(0.U(7.W), "b11111".U, EXE_funct3))
  )) // To Be Modified

  // Control signal - Data Memory
  io.DM_Mem_R := (MEM_opcode===LOAD)
  io.DM_Mem_W := (MEM_opcode===STORE)
  io.DM_Length := Cat(0.U(1.W),MEM_funct3) // length

  // Control signal - Inst Memory
  io.IM_Mem_R := true.B // always true
  io.IM_Mem_W := false.B // always false
  io.IM_Length := "b0010".U // always load a word(inst)

  // Control signal - Scalar Write Back
  io.W_RegWEn := MuxLookup(WB_opcode, false.B, Seq(
    LOAD   ->  true.B,
    STORE  -> false.B,
    BRANCH -> false.B,
    JALR   ->  true.B,
    JAL    ->  true.B,
    OP_IMM ->  true.B,
    OP     ->  true.B,
    AUIPC  ->  true.B,
    LUI    ->  true.B,
    HCF    -> false.B,

    // TA's code
    // OP_IMM -> true.B,
    // LOAD -> true.B,
    // LUI -> true.B,
  ))  // To Be Modified

  io.W_WBSel := MuxLookup(WB_opcode, ALUOUT, Seq(
    LOAD   -> LD_DATA,
    STORE  -> ALUOUT,    // no WB
    BRANCH -> ALUOUT,    // no WB
    JALR   -> PC_PLUS_4,
    JAL    -> PC_PLUS_4,
    OP_IMM -> ALUOUT,
    OP     -> ALUOUT,
    AUIPC  -> ALUOUT,
    LUI    -> ALUOUT,
    HCF    -> ALUOUT,    // no WB

    // TA's code
    // LOAD -> LD_DATA,
  )) // To Be Modified

  // Control signal - Others
  io.Hcf := (WB_opcode === HCF)

  /****************** Data Hazard ******************/
  // Use rs in ID stage
  val is_D_use_rs1 = Wire(Bool())
  val is_D_use_rs2 = Wire(Bool())
  is_D_use_rs1 := MuxLookup(ID_opcode,false.B,Seq(
    LOAD   ->  true.B,
    STORE  ->  true.B,
    BRANCH ->  true.B,
    JALR   ->  true.B,
    JAL    -> false.B,
    OP_IMM ->  true.B,
    OP     ->  true.B,
    AUIPC  -> false.B,
    LUI    -> false.B,
    HCF    -> false.B,

    // TA's code
    // BRANCH -> true.B,
  ))   // To Be Modified
  is_D_use_rs2 := MuxLookup(ID_opcode,false.B,Seq(
    LOAD   -> false.B,
    STORE  ->  true.B,
    BRANCH ->  true.B,
    JALR   -> false.B,
    JAL    -> false.B,
    OP_IMM -> false.B,
    OP     ->  true.B,
    AUIPC  -> false.B,
    LUI    -> false.B,
    HCF    -> false.B,

    // TA's code
    // BRANCH -> true.B,
  ))   // To Be Modified

  // Use rd in EXE stage
  val is_E_use_rd = Wire(Bool())
  is_E_use_rd := MuxLookup(EXE_opcode,false.B,Seq(
    LOAD   ->  true.B,
    STORE  -> false.B,
    BRANCH -> false.B,
    JALR   ->  true.B,
    JAL    ->  true.B,
    OP_IMM ->  true.B,
    OP     ->  true.B,
    AUIPC  ->  true.B,
    LUI    ->  true.B,
    HCF    -> false.B,

    // TA's code
    // OP_IMM -> true.B,
  ))   // To Be Modified

  // Use rd in MEM stage
  val is_M_use_rd = Wire(Bool())
  is_M_use_rd := MuxLookup(MEM_opcode,false.B,Seq(
    LOAD   ->  true.B,
    STORE  -> false.B,
    BRANCH -> false.B,
    JALR   ->  true.B,
    JAL    ->  true.B,
    OP_IMM ->  true.B,
    OP     ->  true.B,
    AUIPC  ->  true.B,
    LUI    ->  true.B,
    HCF    -> false.B,

    // TA's code
    // OP_IMM -> true.B,
  ))   // To Be Modified

  // Use rd in WB stage
  val is_W_use_rd = Wire(Bool())
  is_W_use_rd := MuxLookup(WB_opcode,false.B,Seq(
    LOAD   ->  true.B,
    STORE  -> false.B,
    BRANCH -> false.B,
    JALR   ->  true.B,
    JAL    ->  true.B,
    OP_IMM ->  true.B,
    OP     ->  true.B,
    AUIPC  ->  true.B,
    LUI    ->  true.B,
    HCF    -> false.B,

    // TA's code
    // OP_IMM -> true.B,
  ))   // To Be Modified

  // Hazard condition (rd, rs overlap)
  val is_D_rs1_E_rd_overlap = Wire(Bool())
  val is_D_rs2_E_rd_overlap = Wire(Bool())
  val is_D_rs1_M_rd_overlap = Wire(Bool())
  val is_D_rs2_M_rd_overlap = Wire(Bool())
  val is_D_rs1_W_rd_overlap = Wire(Bool())
  val is_D_rs2_W_rd_overlap = Wire(Bool())

  is_D_rs1_E_rd_overlap := is_D_use_rs1 && is_E_use_rd && (ID_rs1 === EXE_rd) && (EXE_rd =/= 0.U(5.W))
  is_D_rs2_E_rd_overlap := is_D_use_rs2 && is_E_use_rd && (ID_rs2 === EXE_rd) && (EXE_rd =/= 0.U(5.W))
  is_D_rs1_M_rd_overlap := is_D_use_rs1 && is_M_use_rd && (ID_rs1 === MEM_rd) && (MEM_rd =/= 0.U(5.W))
  is_D_rs2_M_rd_overlap := is_D_use_rs2 && is_M_use_rd && (ID_rs2 === MEM_rd) && (MEM_rd =/= 0.U(5.W))
  is_D_rs1_W_rd_overlap := is_D_use_rs1 && is_W_use_rd && (ID_rs1 === WB_rd) && (WB_rd =/= 0.U(5.W))
  is_D_rs2_W_rd_overlap := is_D_use_rs2 && is_W_use_rd && (ID_rs2 === WB_rd) && (WB_rd =/= 0.U(5.W))

  val is_MEM_load = Wire(Bool())
  val is_MEM_store = Wire(Bool())
  val resp_not_back = Wire(Bool())

  is_MEM_load := (MEM_opcode === LOAD)
  is_MEM_store := (MEM_opcode === STORE)
  resp_not_back := !io.DM_Valid && (is_MEM_load || is_MEM_store)

  // Control signal - Stall
  // Stall for Data Hazard
  io.Stall_EXE_ID_DH := (is_D_rs1_E_rd_overlap || is_D_rs2_E_rd_overlap)
  io.Stall_MEM_ID_DH := (is_D_rs1_M_rd_overlap || is_D_rs2_M_rd_overlap)
  io.Stall_WB_ID_DH := (is_D_rs1_W_rd_overlap || is_D_rs2_W_rd_overlap)
  io.Stall_MA := resp_not_back // Stall for Waiting Memory Access
  // Control signal - Flush
  io.Flush_BH := Predict_Miss

  // Control signal - Data Forwarding (Bonus)

  /****************** Data Hazard End******************/
}
