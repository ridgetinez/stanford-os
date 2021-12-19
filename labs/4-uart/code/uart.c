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
    // AUXIRQ
    // An interrupt is pending on an auxillary device.
    struct {
        unsigned auxirq_uart : 1;
        unsigned auxirq_unused : 31;
    };

    // AUXENB
    // Enable an auxillary device's registers for r/w.
    struct {
        unsigned auxenb_uart : 1;
        unsigned auxenb_unused : 31;
    };

    // 56 byte gap (0x5040-0x5008=0x38)
    uint8_t gap[56];

    // AUX_MU_IO_REG
    // ioreg_data is where you can write to the back of the queue,
    // and read from the front of the queue.
    union {
    struct {
        unsigned io_data : 8;
        unsigned io_reserved : 24;
    };
    unsigned ioreg;
    };
    // AUX_MU_IER_REG (unused for now)
    struct {
        unsigned ier_unused;
    };
    // AUX_MU_IIR_REG
    // clear FIFOs for transmit and receive
    struct {
        unsigned iir_interrupt_pending : 1;
        unsigned iir_fifo_clear : 2;
        unsigned iir_gap : 5;
        unsigned iir_reserved : 24;
    };

    // AUX_MU_LCR_REG
    // set UART to 7 bit (clear) or 8 bit (set) mode
    struct {
        unsigned lcr_data_size : 1;
        unsigned lcr_reserved : 31;
    };

    // AUX_MU_MCR_REG (unused)
    struct {
        unsigned mcr_unused;
    };

    // AUX_MU_LSR_REG
    // Check if we can read from the receive FIFO,
    // and if we can write to the transmitting FIFO.
    struct {
        unsigned lsr_rx_data_ready : 1;
        unsigned lsr_rx_overrun : 1;
        unsigned lsr_gap : 3;
        unsigned lsr_tx_data_ready : 1;
        unsigned lsr_tx_overrun : 1;
        unsigned lsr_reserved : 25;
    };

    // AUX_MU_MSR_REG
    // Modem status, unused
    struct {
        unsigned msr_unused;
    };

    // AUX_MU_SCRATCH
    // single byte scratch storage, unused
    struct {
        unsigned scratch_unused;
    };

    // AUX_MU_CNTL_REG
    // control register with extra features
    struct {
        unsigned cntl_rx_enable : 1;
        unsigned cntl_tx_enable : 1;
        unsigned cntl_unused : 30;
    };

    // AUX_MU_STAT_REG
    // Data about the tx/rx FIFO queues.
    struct {
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
    };

    // AUX_MU_BAUD
    // Setting + reading the baud rate.
    struct {
        unsigned baud_rate : 16;
        unsigned baud_reserved : 16;
    };
} muart_registers_t;

// TODO(amartinez): Calling init will set this global variable
static muart_registers_t *uart_registers;

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
}

// TODO(amartinez): how do I use GET32 and PUT32 alongside this the uart register struct?
// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
	return uart_registers->lsr_rx_data_ready;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_getc())
        ;
	return uart_registers->io_data;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
	return uart_registers->lsr_tx_data_ready;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_putc())
        ;
    unsigned tx_fifo_tail = (unsigned) &uart_registers->ioreg;
    PUT32(tx_fifo_tail, c);
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
	return uart_registers->stat_tx_empty;
}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
