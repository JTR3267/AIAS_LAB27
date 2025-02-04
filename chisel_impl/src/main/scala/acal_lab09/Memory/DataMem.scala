package acal_lab09.Memory

import chisel3._
import chisel3.util._
import chisel3.util.experimental.loadMemoryFromFile
import acal_lab09.PiplinedCPU.wide._
import acal_lab09.MemIF._

class DataMem(bits: Int, memLatency: Int = 5) extends Module {
  val io = IO(new MemIF_MEM(addrWidth = bits, dataWidth = 32))

  val memory = Mem(1 << bits, UInt(8.W))
  loadMemoryFromFile(memory, "./src/main/resource/data.hex")

  val sIdle :: sRead :: sWrite :: sWait :: Nil = Enum(4)
  
  val dataReg = RegInit(0.U(32.W))
  val state = RegInit(sIdle.asUInt)
  val counter = RegInit(0.U((math.max(1, log2Ceil(memLatency))).W))
  val readAddrReg  = Reg(UInt(bits.W))
  val writeAddrReg = Reg(UInt(bits.W))
  val writeDataReg = Reg(UInt(32.W))
  val lengthReg    = Reg(UInt(4.W))

  io.rdata := dataReg
  io.Valid := false.B

  switch(state) {
    is(sIdle) {
      when(io.Mem_R) {
        readAddrReg  := io.raddr
        lengthReg    := io.Length
        counter      := 1.U
        state        := sRead
      }.elsewhen(io.Mem_W) {
        writeAddrReg := io.waddr
        writeDataReg := io.wdata
        lengthReg    := io.Length
        counter      := 1.U
        state        := sWrite
      }.otherwise {
        counter      := 0.U
      }

      when(counter === (memLatency.U - 1.U)) {
        state := sIdle
        io.Valid := true.B
      }
    }

    is(sRead) {
      val alignedAddr = MuxLookup(lengthReg, readAddrReg & ~(3.U(bits.W)), Seq(
        Byte -> (readAddrReg),
        Half -> (readAddrReg & ~(1.U(bits.W))),
        Word -> (readAddrReg & ~(3.U(bits.W)))
      ))

      val readDataSInt = MuxLookup(lengthReg, 0.S, Seq(
        Byte  -> memory(alignedAddr).asSInt,
        Half  -> Cat(memory(alignedAddr + 1.U), memory(alignedAddr)).asSInt,
        Word  -> Cat(memory(alignedAddr + 3.U), memory(alignedAddr + 2.U),
                     memory(alignedAddr + 1.U), memory(alignedAddr)).asSInt,
        UByte -> Cat(0.U(24.W), memory(alignedAddr)).asSInt,
        UHalf -> Cat(0.U(16.W), memory(alignedAddr + 1.U),
                     memory(alignedAddr)).asSInt
      ))

      dataReg := readDataSInt.asUInt

      counter := counter + 1.U
      when(counter === (memLatency.U - 1.U)) {
        state := sIdle
        io.Valid := true.B
        counter      := 0.U
      }.otherwise {
        state := sWait
      }
    }

    is(sWrite) {
      val alignedAddr = MuxLookup(lengthReg, writeAddrReg & ~(3.U(bits.W)), Seq(
        Byte -> (writeAddrReg),
        Half -> (writeAddrReg & ~(1.U(bits.W))),
        Word -> (writeAddrReg & ~(3.U(bits.W)))
      ))

      val wd = MuxLookup(lengthReg, 0.U, Seq(
        Byte -> writeDataReg(7, 0),
        Half -> writeDataReg(15, 0),
        Word -> writeDataReg
      ))

      when(lengthReg === Byte) {
        memory(alignedAddr) := wd(7, 0)
      }.elsewhen(lengthReg === Half) {
        memory(alignedAddr)     := wd(7, 0)
        memory(alignedAddr + 1.U) := wd(15, 8)
      }.otherwise { // Word
        memory(alignedAddr)       := wd(7, 0)
        memory(alignedAddr + 1.U) := wd(15, 8)
        memory(alignedAddr + 2.U) := wd(23, 16)
        memory(alignedAddr + 3.U) := wd(31, 24)
      }

      counter := counter + 1.U
      when(counter === (memLatency.U - 1.U)) {
        state := sIdle
        io.Valid := true.B
        counter      := 0.U
      }.otherwise {
        state := sWait
      }
    }

    is(sWait) {
      when(counter === (memLatency.U - 1.U)) {
        state := sIdle
        io.Valid := true.B
        counter      := 0.U
      }.otherwise {
        counter := counter + 1.U
      }
    }
  }
}