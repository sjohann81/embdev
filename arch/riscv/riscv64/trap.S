	.section .text
	.globl _trap_vector
	.globl _restore_context
	.align 4
_trap_vector:
	# Save current context
	# We use mscratch to hold the pointer to the current task's context struct
	csrr t6, mscratch

	sd ra, 8(t6)
	sd sp, 16(t6)
	sd gp, 24(t6)
	sd tp, 32(t6)
	sd t0, 40(t6)
	sd t1, 48(t6)
	sd t2, 56(t6)
	sd s0, 64(t6)
	sd s1, 72(t6)
	sd a0, 80(t6)
	sd a1, 88(t6)
	sd a2, 96(t6)
	sd a3, 104(t6)
	sd a4, 112(t6)
	sd a5, 120(t6)
	sd a6, 128(t6)
	sd a7, 136(t6)
	sd s2, 144(t6)
	sd s3, 152(t6)
	sd s4, 160(t6)
	sd s5, 168(t6)
	sd s6, 176(t6)
	sd s7, 184(t6)
	sd s8, 192(t6)
	sd s9, 200(t6)
	sd s10, 208(t6)
	sd s11, 216(t6)
	sd t3, 224(t6)
	sd t4, 232(t6)
	sd t5, 240(t6)

	# Save the original t6 (x31)
	csrr t5, mscratch
	sd t5, 248(t6)

	# Save control registers
	csrr t0, mepc
	sd t0, 256(t6)
	csrr t0, mstatus
	sd t0, 264(t6)

	# Call C trap handler
	csrr a0, mcause		# pass mcause as argument
	call trap_handler

_restore_context:
    beq a0, zero, _skip_ctx     # default trap returned, just keep tp as is
	mv t6, a0                   # a0 now points to possibly new context
_skip_ctx:
	# Update mscratch for the next interrupt
	csrw mscratch, t6

	# Restore control registers
	ld t0, 256(t6)
	csrw mepc, t0
	ld t0, 264(t6)
	csrw mstatus, t0

	# Restore general purpose registers
	ld ra, 8(t6)
	ld sp, 16(t6)
	ld gp, 24(t6)
	ld tp, 32(t6)
	ld t0, 40(t6)
	ld t1, 48(t6)
	ld t2, 56(t6)
	ld s0, 64(t6)
	ld s1, 72(t6)
	ld a0, 80(t6)
	ld a1, 88(t6)
	ld a2, 96(t6)
	ld a3, 104(t6)
	ld a4, 112(t6)
	ld a5, 120(t6)
	ld a6, 128(t6)
	ld a7, 136(t6)
	ld s2, 144(t6)
	ld s3, 152(t6)
	ld s4, 160(t6)
	ld s5, 168(t6)
	ld s6, 176(t6)
	ld s7, 184(t6)
	ld s8, 192(t6)
	ld s9, 200(t6)
	ld s10, 208(t6)
	ld s11, 216(t6)
	ld t3, 224(t6)
	ld t4, 232(t6)
	ld t5, 240(t6)
	ld t6, 248(t6)

	mret

