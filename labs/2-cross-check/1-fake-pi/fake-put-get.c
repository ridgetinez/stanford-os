// trivial little fake r/pi system that will allow you to debug your
// gpio.c code. 
#include "fake-pi.h"
#include "tape.h"

// main pi-specific thing is a tiny model of device
// memory: for each device address, what happens when you
// read or write it?   in real life you would build this
// model more succinctly with a map, but we write everything 
// out here for maximum obviousness.

#define GPIO_BASE 0x20200000
enum {
    gpio_fsel0 = (GPIO_BASE + 0x00),
    gpio_fsel1 = (GPIO_BASE + 0x04),
    gpio_fsel2 = (GPIO_BASE + 0x08),
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
};

void put32(volatile void *addr, uint32_t v) {
    PUT32((uint32_t)(uint64_t)addr, v);
}

// same, but takes <addr> as a uint32_t
void PUT32(uint32_t addr, uint32_t v) {
    switch(addr) {
    case gpio_lev0:  panic("illegal write to gpio_lev0!\n");
    default: 
        tape_write(addr, v);
    }
    trace("PUT32:%x:%x\n", addr,v);
}

uint32_t get32(const volatile void *addr) {
    return GET32((uint32_t)(uint64_t)addr);
}

/*
 * most <addr> we treat as normal memory: return the 
 * value of the last write.
 *
 * otherwise we do different things: time, status regs,
 * input pins.
 */
uint32_t GET32(uint32_t addr) {
    unsigned v;

    switch(addr) {
    // to fake a changing environment, we want gpio_lev0 to 
    // change --- we just use a uniform random coin toss, 
    // but you would bias these as well or make them more 
    // realistic by reading from a trace from a run on 
    // the raw hardware, correlating with other pins or 
    // time or ...
    case gpio_lev0:  v = fake_random();  break;
    default: 
        // return value of last write, or random if this is the first read.
        v = tape_get(addr);
        break;
    }
    trace("GET32:%x:%x\n", addr,v);
    return v;
}
