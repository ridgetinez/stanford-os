#include <stdlib.h>
#include "fake-random.h"
#include "tape.h"

static tape_t *new_tape_node(uint32_t addr, uint32_t v) {
    tape_t *n = malloc(sizeof(tape_t));
    n->addr = addr; n->val = v; n->next = 0x0;
    return n;
}

uint32_t tape_get(uint32_t addr) {
    tape_t *curr = tape;
    for (; curr != 0x0; curr = curr->next) {
        if (curr->addr == addr) break;
    }
    return (curr == 0x0) ? tape_write(addr, fake_random()) : curr->val;
}

uint32_t tape_write(uint32_t addr, uint32_t v) {
    if (tape == 0x0) {
        tape = new_tape_node(addr, v);
        return v;
    } else if (tape->addr == addr) {
        tape->val = v;
        return v;
    }

    // precondition: tape is non-nil, head address has been checked.
    // postcondition: all tape node addresses have been checked
    tape_t *curr = tape;
    for (; curr->next != 0x0; curr = curr->next) {
        if (curr->next->addr == addr) {
            tape->val = v;
            return v;
        }
    }
    // precondition: all nodes checked, no addr found. 
    // postcondition: same tape, with addr within the tape.
    // note: due to time locality, there's an argument
    // to instead prepend this so that future gets and writes
    // don't have to traverse the entire tape.
    curr->next = new_tape_node(addr, v);
    return v;
}