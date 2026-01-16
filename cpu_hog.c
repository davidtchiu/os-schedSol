/*
 * cpu_hog.c
 *
 * CPU-bound infinite loop.
 *
 */

#include <stdint.h>

int main(void) {
    // Use volatile so the compiler cannot optimize the loop away at -O2/-O3.
    volatile uint64_t x = 0x123456789ABCDEF0ULL;

    for (;;) {
        // Simple arithmetic to keep the core busy.
        x = x * 1664525ULL + 1013904223ULL;
    }
}
