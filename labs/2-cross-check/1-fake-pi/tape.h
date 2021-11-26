#pragma once 

#include <stdint.h>

// tape_t mocks pi's memory.
// writes add to the tape, and gets fetch the latest associated value.
typedef struct tape { 
    uint32_t addr; 
    uint32_t val; 
    struct tape *next;
} tape_t;

// global tape variable, library functions operate over this tape.
tape_t *tape;

// tape_get gets the latest value at `addr`.
// if no such addr exists, commits the addr to a random value.
uint32_t tape_get(uint32_t addr);

// tape_write records `v` as the latest value of `addr`.
// returns the value being written `v`.
uint32_t tape_write(uint32_t addr, uint32_t v);