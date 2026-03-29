    .section .text.init
	.globl _trap_vector
	.globl _restore_context
	.align 4
_trap_vector:
    nop                         # this must be a NOP
    sw ra, 4(tp)                # save registers
    sw sp, 8(tp)
    sw gp, 12(tp)
    sw t0, 20(tp)
    sw t1, 24(tp)
    sw t2, 28(tp)
    sw s0, 32(tp)
    sw s1, 36(tp)
    sw a0, 40(tp)
    sw a1, 44(tp)
    sw a2, 48(tp)
    sw a3, 52(tp)
    sw a4, 56(tp)

    li a5, 0xf0000040           # read IRQ_EPC
    lw a5, 0(a5)
    addi a5, a5, -4             # last inst was not commited on pipeline
    sw a5, 64(tp)               # save EPC
    
    lui a1, 0xf0000
    lw a0, 0x10(a1)             # read IRQ_CAUSE
    lw a2, 0x20(a1)             # read IRQ_MASK
    and a0, a0, a2              # mask CAUSE
    jal ra, trap_handler         # jump to C interrupt handler

_restore_context:
    beq a0, zero, _skip_ctx     # default trap returned, just keep tp as is
	mv tp, a0                   # a0 now points to possibly new context
_skip_ctx:
    lw ra, 4(tp)                # restore registers
    lw sp, 8(tp)
    lw gp, 12(tp)
    lw t0, 20(tp)
    lw t1, 24(tp)
    lw t2, 28(tp)
    lw s0, 32(tp)
    lw s1, 36(tp)
    lw a0, 40(tp)
    lw a1, 44(tp)
    lw a2, 48(tp)
    lw a3, 52(tp)
    lw a4, 56(tp)

    li a5, 0xf0000030           # enable interrupts
    ori a5, a5, 0x1
    sb a5, 0(a5)
    
    lw a5, 64(tp)               # restore EPC
    jalr zero, a5               # context restored, continue
    
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
