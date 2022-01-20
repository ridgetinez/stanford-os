#include "rpi.h"
#include "sw-uart.h"

// helper: cleans up the code.
static inline void timed_write(int pin, int v, unsigned usec) {
    gpio_write(pin,v);
    delay_us(usec);
}

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_putc(sw_uart_t *uart, unsigned char c) {
    timed_write(uart->tx, 0, uart->usec_per_bit);
    for (int i = 0; i < 8; i++) {
        timed_write(uart->tx, (c >>= 1) & 1, uart->usec_per_bit); 
    }
    timed_write(uart->tx, 1, uart->usec_per_bit);
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_getc(sw_uart_t *uart, int timeout_usec) {
    unsigned ra, rb = timer_get_usec();
    while (gpio_read(uart->rx) != 0) {
	if ((ra = timer_get_usec) - rb >= timeout_usec) return -1;
    }

    // delay for half a period to get the middle of a bit transmission
    unsigned initial_wait = uart->usec_per_bit + (uart->usec_per_bit >> 1);
    delay_us(initial_wait);
	

    int c = 0, mask = 0b1;
    for (int i = 0; i < 8; i++, mask <<= 1) {
	if (gpio_read(uart->rx)) c &= mask;
	delay_us(uart->usec_per_bit);
    }
    return c;
}

void sw_uart_putk(sw_uart_t *uart, const char *msg) {
    for(; *msg; msg++)
        sw_uart_putc(uart, *msg);
}
