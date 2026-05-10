.section .text.init
.globl _start
_start:
	# Disable all interrupts initially
	csrw mie, zero
	csrw mstatus, zero

	# Set up stack pointer
	la sp, _stack_top

	# Clear BSS
	la t0, _bss_start
	la t1, _bss_end
    
_clear_bss:
	bgeu t0, t1, _clear_bss_done
	sd zero, 0(t0)
	addi t0, t0, 8
	j _clear_bss
    
_clear_bss_done:
	# Set trap vector
	la t0, _trap_vector
	csrw mtvec, t0
    
    # Grant U-mode full access to all memory (NAPOT mode)
    li t0, -1
    csrw pmpaddr0, t0
    li t0, 0x0f       # PMP_R (1) | PMP_W (2) | PMP_X (4) | PMP_NAPOT (8)
    csrw pmpcfg0, t0
    
    # set up mscratch for context saving on interrupts
    la t0, default_trap_frame
    csrw mscratch, t0
	
    # enable global interrupts
    csrs mstatus, 8

    # call bsp init
    jal ra, bsp_init
    
	# Jump to main
	call main

	# If main returns, loop forever
_hang:
	wfi
	j _hang

