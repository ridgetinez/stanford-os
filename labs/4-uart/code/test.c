#include <stdio.h>

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

int main(void) {
    unsigned v = 2;
    statreg_t *statreg = (statreg_t *) &v;
    printf("%d %d %d\n", statreg->stat_rx_available, statreg->stat_tx_available, statreg->stat_rx_idle);
    return 0;
}