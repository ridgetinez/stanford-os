/*
 * interrupt-asm.s
 *
 * Code for interrupt handling.  Refer to armisa.pdf in docs/ for what
 * the opcodes mean.
 */
#include "rpi-asm.h"

/*
 * Enable/disable interrupts.
 *
 * CPSR = current program status register
 *        upper bits are different carry flags.
 *        lower 8:
 *           7 6 5 4 3 2 1 0
 *          +-+-+-+---------+
 *          |I|F|T|   Mode  |
 *          +-+-+-+---------+
 *
 *  I : disables IRQ when = 1.
 *  F : disables FIQ when = 1.
 *  T : = 0 indicates ARM execution, = 1 is thumb execution.
 *      Mode = current mode.
 */
.globl system_enable_interrupts
system_enable_interrupts:
    mrs r0,cpsr         @ move process status register (PSR) to r0
    bic r0,r0,#(1<<7)	@ clear 7th bit.
    msr cpsr_c,r0		@ move r0 back to PSR
    bx lr		        @ return.

.globl system_disable_interrupts
system_disable_interrupts:
    mrs r0,cpsr		       
    orr r0,r0,#(1<<7)	       @ set 7th bit: or in 0b100 0000
    msr cpsr_c,r0
    bx lr

@ .globl makes a symbol available to the linker
@ this means that the symbol can be used within
@ other C files. e.g. extern unsigned _interrupt_table
@ in interrupts-c.c
.globl _interrupt_table
.globl _interrupt_table_end
@ interrupt table to handle different exceptions
@ look at armv6-interrupts A2-16 
@ Q(amartinez): is the order the handlers are in
@ defined by the architecture, or by the OS?
@ A: Doesn't make sense for it to be done by the OS,
@ Since the processor is the first place that will see
@ the exception and dispatches to the exception handler.
@ This can be confirmed if we see a place where this
@ table is registered somewhere? Like, how does the processor
@ know where to look for the exception handlers?
@ Also, the order matches the order defined in A2-16
_interrupt_table:
  ldr pc, _reset_asm
  ldr pc, _undefined_instruction_asm
  ldr pc, _software_interrupt_asm
  ldr pc, _prefetch_abort_asm
  ldr pc, _data_abort_asm
  ldr pc, _reset_asm @ Q(amartinez): why have another _reset_asm here? What exception type does this index correspond to?
                     @ A: it's because each instruction is 0x4 size and data abort starts at 0x10 when interrupt starts at 0x18.
                     @    if you added the interrupt instruction directly after, it would be placed at 0x14 instead of 0x18 so we pad.
  ldr pc, _interrupt_asm
fast_interrupt_asm:
  sub   lr, lr, #4 @First instr of FIQ handler
  push  {lr}
  push  {r0-r12}
  mov   r0, lr              @ Pass old pc
  bl    fast_interrupt_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

_reset_asm:                   .word reset_asm
_undefined_instruction_asm:   .word undefined_instruction_asm
_software_interrupt_asm:      .word software_interrupt_asm
_prefetch_abort_asm:          .word prefetch_abort_asm
_data_abort_asm:              .word data_abort_asm
_interrupt_asm:               .word interrupt_asm
@ Q(amartinez): Why is _interrupt_table_end defined here, and not after _interrupt_table?
_interrupt_table_end:

@ only handler that should run since we only enable general interrupts
interrupt_asm:
  @ where do we put the stack of the place we came from?
  @ this looks like it's discarding their stack
  mov sp, #INT_STACK_ADDR   @ i believe we have 512mb - 16mb, so this should be safe
  @ https://stackoverflow.com/questions/19909410/setting-irq-handler-in-arm-assembly
  sub   lr, lr, #4

  push  {r0-r12,lr}         @ XXX: pushing too many registers: only need caller
  @ vpush {s0-s15}	        @ uncomment if want to save caller-saved fp regs

  mov   r0, lr              @ Pass old pc
  bl    interrupt_vector    @ C function

  @ vpop {s0-s15}           @ pop caller saved fp regs
  pop   {r0-r12,lr} 	    @ pop integer registers

  @ return from interrupt handler: will re-enable general ints.
  @ Q: what happens if you do "mov" instead?
  @ A: If you used mov, the cpsr would still be in the processor mode
  @    that the exception ran in, which might be more permissive than
  @    you want. e.g. you don't want to be in supervisor mode when
  @    returning back to user code as a malicious process would be permitted
  @    to do anything it would like.
  movs    pc, lr        @ moves the link register into the pc and implicitly
                        @ loads the PC with the result, then copies the 
                        @ SPSR to the CPSR.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ we don't generate any of these, will just panic and halt.
@
@ you can override these!
reset_asm:
  sub   lr, lr, #4
  bl    reset_vector
undefined_instruction_asm:
  sub   lr, lr, #4
  bl    undefined_instruction_vector
software_interrupt_asm:
  sub   lr, lr, #4
  bl    software_interrupt_vector
prefetch_abort_asm:
  sub   lr, lr, #4
  bl    prefetch_abort_vector
data_abort_asm:
  sub   lr, lr, #4
  bl    data_abort_vector


@ test that saving using the caret gives the right value.
    # stmfd r0, {lr}^
    # stmia r0, {lr}^
    # pop {r0}

store_shadow_lr:
.globl store_shadow_lr
    stm r0, {lr}^
    bx lr

@ simple demonstration of how to switch modes to get the
@ banked lr.
#define SUPER_MODE      0b10011
get_shadow_lr:
.globl get_shadow_lr
    mrs r1, cpsr
    msr cpsr_c, #SUPER_MODE
    mov r0, lr
    msr cpsr_c, r1
    bx lr
