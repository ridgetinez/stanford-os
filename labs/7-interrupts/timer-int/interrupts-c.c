#include "rpi.h"
#include "rpi-interrupts.h"

// initialize global interrupt state.
void int_init(void) {
    // put interrupt flags in known state. 
    //  BCM2835 manual, section 7.5 , 112
    // Disabling IRQ1 and IRQ2 as they correspond to
    // GPU interrupts, and not the processor / simple 
    // interrupts.
    PUT32(Disable_IRQs_1, 0xffffffff);
    PUT32(Disable_IRQs_2, 0xffffffff);
    // Why do we need the dev_barrier here?
    // A: Ensure that the write to the two registers
    //    have been issued before continuing.
    dev_barrier();
    // Q(amartinez): Why don't we enable IRQ basic here?
    // The enable IRQ basic register controls many interrupts,
    // like mailbox, gpu 1 and 0 ints, and timers. At this point
    // in time all we know is that we'd like to disable IRQ 1 and 2
    // as they're GPU interrupts, but we don't know what basic interrupts
    // to enable. That'll happen later down the track.


    /*
     * Copy in interrupt vector table and FIQ handler _table and _table_end
     * are symbols defined in the interrupt assembly file, at the beginning
     * and end of the table and its embedded constants.
     */
    extern unsigned _interrupt_table;
    extern unsigned _interrupt_table_end;

    // where the interrupt handlers go.
    // Q(amartinez): Why put the handlers at 0x0? Is it because we know this page is free?
    // Q(amartinez): Why not use _interrupt_table directly vs. copying their contents to 0x0?
    // A: armv6-interrupts A2-16 interrupt table is at 0x0 for ARMv6
    // Note(amartinez): We don't have a pagetable currently, but I presume the kernel pagetable
    // will be the only place that we'd need to do this copy in under the assumption that when
    // an exception is created, it looks at the kernel pagetable and not the pagetable for the
    // current execution context.
#   define RPI_VECTOR_START  0
    unsigned *dst = (void*)RPI_VECTOR_START,
                 *src = &_interrupt_table,
                 n = &_interrupt_table_end - src;
    for(int i = 0; i < n; i++)
        dst[i] = src[i];
}

#define UNHANDLED(msg,r) \
	panic("ERROR: unhandled exception <%s> at PC=%x\n", msg,r)
void fast_interrupt_vector(unsigned pc) {
	UNHANDLED("fast", pc);
}

// this is used for syscalls.
// interrupts-asm.S handles the normal interrupt exception type
// with a proper exception handler. These handlers are ones we'll
// deal with in future labs (e.g. maybe page faults first start here?)
void software_interrupt_vector(unsigned pc) {
	UNHANDLED("soft interrupt", pc);
}
void reset_vector(unsigned pc) {
	UNHANDLED("reset vector", pc);
}
void undefined_instruction_vector(unsigned pc) {
	UNHANDLED("undefined instruction", pc);
}
void prefetch_abort_vector(unsigned pc) {
	UNHANDLED("prefetch abort", pc);
}
void data_abort_vector(unsigned pc) {
	UNHANDLED("data abort", pc);
}
