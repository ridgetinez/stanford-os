// implement:
//  void uart_init(void)
//
//  int uart_can_getc(void);
//  int uart_getc(void);
//
//  int uart_can_putc(void);
//  void uart_putc(unsigned c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

#define AUX_BASE 0x20215000

static const unsigned aux_start = (AUX_BASE);

// TODO(amartinez): Might be better to define specific structs
// for each register so that I can GET32 a whole word, and then
// cast the value into the specific register struct to make use
// of the bitfield mapping. Modify the bitfield struct locally,
// and the PUT32 the final word! :)
typedef struct muart_registers {
    unsigned auxirq;
    unsigned auxenb;
    // 56 byte gap (0x5040-0x5008=0x38)
    uint8_t gap[56];
    unsigned ioreg;
    unsigned ierreg;
    unsigned iirreg;
    unsigned lcrreg;
    unsigned mcrreg_unused;
    unsigned lsrreg;
    unsigned msrreg_unused;
    unsigned scratchreg_unused;
    unsigned cntlreg;
    unsigned statreg;
    unsigned baudreg;
} muart_registers_t;

// AUXIRQ
// An interrupt is pending on an auxillary device.
typedef struct auxirq {
    unsigned uart : 1;
    unsigned auxirq_unused : 31;
} auxirq_t;

// AUXENB
// Enable an auxillary device's registers for r/w.
typedef struct auxenb {
    unsigned auxenb_uart : 1;
    unsigned auxenb_unused : 31;
} auxenb_t;

// AUX_MU_IO_REG
// ioreg_data is where you can write to the back of the queue,
// and read from the front of the queue.
typedef struct ioreg {
    unsigned io_data : 8;
    unsigned io_reserved : 24;
} ioreg_t;

// AUX_MU_IER_REG
// enable/disable interrupts
typedef struct ierreg {
    unsigned ier_tx_interrupt : 1;
    unsigned ier_rx_interrupt : 1;
    unsigned ier_unused : 30;
} ierreg_t;

// AUX_MU_IIR_REG
// clear FIFOs for transmit and receive
typedef struct iireg {
    unsigned iir_interrupt_pending : 1;
    unsigned iir_rx_fifo_clear : 1;
    unsigned iir_tx_fifo_clear : 1;
    unsigned iir_gap : 5;
    unsigned iir_reserved : 24;
} iireg_t;

// AUX_MU_LCR_REG
// set UART to 7 bit (clear) or 8 bit (set) mode
typedef struct lcrreg {
    unsigned lcr_data_size : 1;
    unsigned lcr_reserved : 31;
} lcrreg_t;

// AUX_MU_LSR_REG
// Check if we can read from the receive FIFO,
// and if we can write to the transmitting FIFO.
typedef struct lsrreg {
    unsigned lsr_rx_data_ready : 1;
    unsigned lsr_rx_overrun : 1;
    unsigned lsr_gap : 3;
    unsigned lsr_tx_data_ready : 1;
    unsigned lsr_tx_overrun : 1;
    unsigned lsr_reserved : 25;
} lsrreg_t;

// AUX_MU_CNTL_REG
// control register with extra features
typedef struct cntlreg {
    unsigned cntl_rx_enable : 1;
    unsigned cntl_tx_enable : 1;
    unsigned cntl_unused : 30;
} cntlreg_t;

// AUX_MU_STAT_REG
// Data about the tx/rx FIFO queues.
typedef struct statreg {
    unsigned stat_rx_available : 1;
    unsigned stat_tx_available : 1;
    unsigned stat_rx_idle : 1;
    unsigned stat_tx_idle : 1;
    unsigned stat_rx_overrun : 1;
    unsigned stat_tx_full : 1;
    unsigned stat_rts : 1;
    unsigned stat_cts : 1;
    unsigned stat_tx_empty : 1;
    unsigned stat_tx_done : 1; // stat_tx_idle & stat_tx_empty
    unsigned stat_unused : 22;
} statreg_t;

// AUX_MU_BAUD
// Setting + reading the baud rate.
typedef struct baudreg {
    unsigned baud_rate : 16;
    unsigned baud_reserved : 16;
} baudreg_t;

// TODO(amartinez): Calling init will set this global variable
static muart_registers_t *uart_registers;

// helper functions
int uart_tx_is_idle(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->statreg);
    statreg_t *statreg = (statreg_t *) &v;
	return statreg->stat_tx_idle;
}

void uart_set_tx(int set) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->cntlreg);
    cntlreg_t *cntlreg = (cntlreg_t *) &v;
    cntlreg->cntl_tx_enable = set;
    PUT32((unsigned) &uart_registers->cntlreg, *((unsigned *) cntlreg));
}

void uart_set_rx(int set) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->cntlreg);
    cntlreg_t *cntlreg = (cntlreg_t *) &v;
    cntlreg->cntl_rx_enable = set;
    PUT32((unsigned) &uart_registers->cntlreg, *((unsigned *) cntlreg));
}

void uart_set_tx_interrupt(int set) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->ierreg);
    ierreg_t *ierreg = (ierreg_t *) &v;
    ierreg->ier_tx_interrupt = set;
    PUT32((unsigned) &uart_registers->ierreg, *((unsigned *) ierreg));
}

void uart_set_rx_interrupt(int set) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->ierreg);
    ierreg_t *ierreg = (ierreg_t *) &v;
    ierreg->ier_rx_interrupt = set;
    PUT32((unsigned) &uart_registers->ierreg, *((unsigned *) ierreg));
}

void uart_set_baudrate(int baudrate) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->baudreg);
    baudreg_t *baudreg = (baudreg_t *) &v;
    baudreg->baud_rate = baudrate;
    PUT32((unsigned) &uart_registers->baudreg, *((unsigned *) baudreg));
}

// TODO: turn this GET32 -> PUT32 into a macro
// set_uart_reg(reg, field, val)
void uart_set_8n1(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->lcrreg);
    lcrreg_t *lcrreg = (lcrreg_t *) &v;
    lcrreg->lcr_data_size = 1;
    PUT32((unsigned) &uart_registers->lcrreg, *((unsigned *) lcrreg));
}

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    // set gpio pins 14/15 as tx/rx respectively.
    gpio_set_function(14, GPIO_FUNC_ALT5); // tx
    gpio_set_function(15, GPIO_FUNC_ALT5); // rx

    // turn on the uart in aux
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v_auxenb = GET32((unsigned) &uart_registers->auxenb);
    auxenb_t *auxenb = (auxenb_t *) &v_auxenb;
    auxenb->auxenb_uart = 1;
    PUT32((unsigned) &uart_registers->auxenb, *((unsigned *) auxenb));

    // disable current tx/rx
    uart_set_rx(0); uart_set_tx(0);

    // // disable interrupts
    uart_set_rx_interrupt(0); uart_set_tx_interrupt(0);

    // // clear FIFO queues
    uart_clear_rx(); uart_clear_tx();

    // configure 115200 baud rate
    // took core_freq = 250 to mean 250MHz and used the formula to get 270.2
    // Tested from 250-300, looks like it's pretty sensitive as it didn't work
    // at 250, nor 300. Why doesn't it work at other baudrate reg values?
    uart_set_baudrate(270);
    
    // // configure 8n1
    uart_set_8n1();

    // enable tx/rx
    uart_set_rx(1); uart_set_tx(1);
}

// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->lsrreg);
    lsrreg_t *lsrreg = (lsrreg_t *) &v;
	return lsrreg->lsr_rx_data_ready;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_getc())
        ;
    unsigned v = GET32((unsigned) &uart_registers->ioreg);
    ioreg_t *ioreg = (ioreg_t *) &v;
	return ioreg->io_data;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->lsrreg);
    lsrreg_t *lsrreg = (lsrreg_t *) &v;
	return lsrreg->lsr_tx_data_ready;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_putc())
        ;
    PUT32((unsigned) &uart_registers->ioreg, c);
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int uart_has_data(void) {
    return uart_can_getc();
}

// return -1 if no data, otherwise the byte.
int uart_getc_async(void) {
    if(!uart_has_data())
        return -1;
    return uart_getc();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->statreg);
    statreg_t *statreg = (statreg_t *) &v;
	return statreg->stat_tx_empty;
}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
