/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32 
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
static const unsigned gpio_sel0 = (GPIO_BASE);
static const unsigned gpio_set0 = (GPIO_BASE + 0x1C);
static const unsigned gpio_clr0 = (GPIO_BASE + 0x28);
static const unsigned gpio_lev0 = (GPIO_BASE + 0x34);

//  helper function to set the function select register for <pin>.
void gpio_set_function(unsigned pin, unsigned fv) {
    uint32_t address = gpio_sel0 + (pin/10 * sizeof(unsigned));
    uint32_t current = GET32(address);
    uint32_t shift = 3 * (pin % 10);
    PUT32(address, (current & ~(0b111 << shift)) | (fv << shift));
}

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.  
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you 
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    gpio_set_function(pin, 0b001);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    PUT32(gpio_set0, 0b1 << pin);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    PUT32(gpio_clr0, 0b1 << pin);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
       gpio_set_on(pin);
    else
       gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    gpio_set_function(pin, 0x000);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
    // rpi zero w only has 21 GPIO pins,
    // all contained in word at gpio_lev0
    return GET32(gpio_lev0) & (0b1 << pin);
}
