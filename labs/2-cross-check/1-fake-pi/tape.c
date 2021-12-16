#include <stdlib.h>
#include "fake-random.h"
#include "tape.h"

static tape_t *new_tape_node(uint32_t addr, uint32_t v) {
    tape_t *n = malloc(sizeof(tape_t));
    n->addr = addr; n->val = v; n->next = 0x0;
    return n;
}

// returns pointer to node if addr found, otherwise null.
static tape_t *tape_find(uint32_t addr) {
    tape_t *curr = tape;
    for (; curr != 0x0; curr = curr->next) {
        if (curr->addr == addr) break;
    }
    return curr;
}

static uint32_t tape_prepend(uint32_t addr, uint32_t v) {
    tape_t *n = new_tape_node(addr, v);
    if (tape == 0x0) {
        tape = n;
    } else {
        n->next = tape;
        tape = n;
    }
    return v;
}

uint32_t tape_get(uint32_t addr) {
    tape_t *n = tape_find(addr);
    return (n == 0x0) ? tape_prepend(addr, fake_random()) : n->val;
}

uint32_t tape_write(uint32_t addr, uint32_t v) {
    tape_t *curr = tape;
    tape_t *n = tape_find(addr);
    return (n == 0x0) ? tape_prepend(addr, v) : (n->val = v, v);
}