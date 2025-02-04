addi x0, x0, 0
lui x06, 0x00000008
addi x06, x06, 0x00000000
addi t2, x0, 0x55
sb t2, 0(t1)
lw t3, 0(t1)
addi t4, x0, 0x55
beq t3, t4, pass1
addi a0, x0, 1
beq x0, x0, fail
beq x0, x0, test2
addi t1, x0, 5
addi x0, x0, 0
addi x0, x0, 0
add t2, t1, t1
addi x0, x0, 0
addi x0, x0, 0
add t3, t2, x0
addi t4, x0, 10
beq t3, t4, pass2
addi a0, x0, 2
beq x0, x0, fail
beq x0, x0, test3
addi t1, x0, 5
addi x0, x0, 0
add t2, t1, t1
addi x0, x0, 0
add t3, t2, x0
addi t4, x0, 10
beq t3, t4, pass3
addi a0, x0, 3
beq x0, x0, fail
beq x0, x0, test4
addi t1, x0, 5
add t2, t1, t1
add t3, t2, x0
addi t4, x0, 10
beq t3, t4, pass4
addi a0, x0, 4
beq x0, x0, fail
beq x0, x0, test5
addi t1, x0, 1
add t1, t1, t1
add t1, t1, t1
add t1, t1, t1
add t2, t1, x0
addi t3, x0, 8
beq t2, t3, pass5
addi a0, x0, 5
beq x0, x0, fail
beq x0, x0, test6
lui x06, 0x00000008
addi x06, x06, 0x00000004
addi t2, x0, 0x5a
sb t2, 0(t1)
lw t3, 0(t1)
add t4, t3, x0
addi t5, x0, 0x5a
beq t4, t5, pass6
addi a0, x0, 6
beq x0, x0, fail
beq x0, x0, test7
addi t1, x0, 0
addi t2, x0, 0
addi t3, x0, 0
beq t1, t2, flush7
addi t3, x0, 999
addi t4, x0, 111
beq t3, x0, correct7
addi a0, x0, 7
beq x0, x0, fail
addi t5, x0, 111
beq t4, t5, pass7
addi a0, x0, 7
beq x0, x0, fail
beq x0, x0, test8
addi t1, x0, 10
jal t1, flush8
add t1, t1, t1
addi t1, t1, 5
addi t1, x0, 10
add t2, t1, x0
addi t3, x0, 10
beq t2, t3, pass8
addi a0, x0, 8
beq x0, x0, fail
beq x0, x0, test9
lui x06, 0x00000008
addi x06, x06, 0x00000008
lw t2, 0(t1)
beq t2, t2, flush9
add t2, t2, t2
addi t2, t2, 1
add t3, t2, x0
addi t4, x0, 0x66
beq t3, t4, pass9
addi a0, x0, 9
beq x0, x0, fail
addi a0, x0, 0
beq x0, x0, exit
beq x0, x0, exit
hcf
nop
nop
nop
nop
