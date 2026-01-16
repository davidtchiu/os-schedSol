/*
 * interactive.c
 *
 * Sleeps for CLICK_INTERVAL_MS between "clicks" and reports:
 *   - elapsed time since start (seconds, 3 decimals)
 *   - instantaneous jitter for THIS click (ms, 3 decimals)
 *
 * jitter_ms = actual_interval_ms - CLICK_INTERVAL_MS
 *
 * Usage:
 *   ./interactive <runtime_seconds>
 *
 * Build:
 *   gcc -O2 -Wall -Wextra -std=c11 interactive.c -o interactive
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CLICK_INTERVAL_MS 300.0

double now_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime");
        exit(1);
    }
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <runtime_seconds>\n", argv[0]);
        return 1;
    }

    int runtime_seconds = atoi(argv[1]);
    if (runtime_seconds <= 0) {
        fprintf(stderr, "runtime_seconds must be > 0\n");
        return 1;
    }

    // Line-buffer so each printf appears promptly in a terminal.
    setvbuf(stdout, NULL, _IOLBF, 0);

    double start_ms = now_ms();
    double prev_click_ms = start_ms;

    for (;;) {
        // Sleep for ~300ms to simulate waiting for user input.
        usleep((useconds_t)(CLICK_INTERVAL_MS * 1000.0));

        double now = now_ms();

        double elapsed_ms = now - start_ms;
        double elapsed_s = elapsed_ms / 1000.0;

        // Stop once we've reached runtime_seconds.
        if (elapsed_s >= (double)runtime_seconds) {
            break;
        }

        // Instantaneous jitter for this click only.
        double actual_interval_ms = now - prev_click_ms;
        double jitter_ms = actual_interval_ms - CLICK_INTERVAL_MS;

        printf("[click] elapsed=%.3f ms  jitter=%.3f ms\n", actual_interval_ms, jitter_ms);

        prev_click_ms = now;
    }

    return 0;
}
