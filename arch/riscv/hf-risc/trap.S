    .section .text.init
    .global _isr
_isr:
    nop                      # this must be a NOP
    addi sp, sp, -40
    sw ra, 0(sp)
    sw t0, 4(sp)
    sw t1, 8(sp)
    sw t2, 12(sp)
    sw a0, 16(sp)
    sw a1, 20(sp)
    sw a2, 24(sp)
    sw a3, 28(sp)
    sw a4, 32(sp)

    li a5, 0xf0000040        # read IRQ_EPC
    lw a5, 0(a5)
    addi a5, a5, -4          # rollback, last opcode (at EPC) was not commited
    sw a5, 36(sp)
    
    lui a1, 0xf0000
    lw a0, 0x10(a1)          # read IRQ_CAUSE
    lw a2, 0x20(a1)          # read IRQ_MASK
    and a0, a0, a2           # pass CAUSE and MASK and the stack pointer to the C handler
    addi a1, sp, 0

    jal ra, irq_handler      # jump to C handler

    lw ra, 0(sp)
    lw t0, 4(sp)
    lw t1, 8(sp)
    lw t2, 12(sp)
    lw a0, 16(sp)
    lw a1, 20(sp)
    lw a2, 24(sp)
    lw a3, 28(sp)
    lw a4, 32(sp)

    li a5, 0xf0000030
    ori a5, a5, 0x1
    addi sp, sp, 40
    sb a5, 0(a5)
    lw a5, -4(sp)           # 36(sp)

    jalr zero, a5           # context restored, continue

    .global _interrupt_set
_interrupt_set:
    li a1, 0xf0000030
    lw a2, 0(a1)
    sw a0, 0(a1)
    addi a0, a2, 0
    ret

    .global setjmp
setjmp:
    sw s0, 0(a0)
    sw s1, 4(a0)
    sw gp, 8(a0)
    sw tp, 12(a0)
    sw sp, 16(a0)
    sw ra, 20(a0)
    ori a0, zero, 0
    ret

    .global longjmp
longjmp:
    lw s0, 0(a0)
    lw s1, 4(a0)
    lw gp, 8(a0)
    lw tp, 12(a0)
    lw sp, 16(a0)
    lw ra, 20(a0)
    ori a0, a1, 0
    ret

    .global syscall
syscall:
    ecall
    ret
