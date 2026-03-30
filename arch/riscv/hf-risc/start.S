    .section .text.init
    .globl _start
_start:
    # set up stack pointer and clear .bss
    la sp, _stack_top
    la t0, _bss_start
    la t1, _bss_end

_bss_clear:
    # clear the .bss
    sw zero, 0(t0)
    addi t0, t0, 4
    blt t0, t1, _bss_clear

    # configure IRQ_VECTOR
    la t0, _trap_vector
    li t1, 0xf0000000
    sw t0, 0(t1)

    # set up tp for context saving on interrupts
    la tp, default_trap_frame

    # set up and enable global interrupts
    jal ra, irq_enable
    
    # call bsp init
    jal ra, bsp_init

    # jump to main
    jal ra, main

    # this will interrupt the simulation (assertion)
    li t0, 0xe0000000
    sw a0, 0(t0)
_l1:
    beq zero, zero, _l1
