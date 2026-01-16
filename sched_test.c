/*
 * sched_test.c
 *
 * Controller:
 *   ./sched_test <num-hogs> <runtime-seconds>
 *
 * Required behavior:
 *   1) fork+exec CPU hogs first
 *   2) fork+exec interactive last (passing runtime-seconds)
 *   3) waitpid() for interactive process
 *   4) kill hogs
 *   5) reap remaining children
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void exec_or_die(const char *path, char *const argv[]) {
    execv(path, argv);
    perror("execv");
    _exit(127);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_hogs> <runtime_seconds>\n", argv[0]);
        return 1;
    }

    int num_hogs = atoi(argv[1]);
    int runtime_seconds = atoi(argv[2]);

    if (num_hogs < 0) {
        fprintf(stderr, "num_hogs must be >= 0\n");
        return 1;
    }
    if (num_hogs > 256) {
        fprintf(stderr, "num_hogs is too large!\n");
        return 1;
    }
    if (runtime_seconds <= 0) {
        fprintf(stderr, "runtime_seconds must be > 0\n");
        return 1;
    }

    pid_t *hog_pids = NULL;
    if (num_hogs > 0) {
        hog_pids = malloc((size_t)num_hogs * sizeof(*hog_pids));
        if (!hog_pids) {
            perror("malloc");
            return 1;
        }
    }

    // 1) Spawn hogs first
    for (int i = 0; i < num_hogs; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            // Best-effort cleanup of already launched hogs
            for (int j = 0; j < i; j++) kill(hog_pids[j], SIGTERM);
            free(hog_pids);
            return 1;
        }

        if (pid == 0) {
            char *child_argv[] = { (char *)"./cpu_hog", NULL };
            exec_or_die("./cpu_hog", child_argv);
        }

        hog_pids[i] = pid;
    }

    // 2) Spawn interactive last
    pid_t interactive_pid = fork();
    if (interactive_pid < 0) {
        perror("fork");
        for (int i = 0; i < num_hogs; i++) kill(hog_pids[i], SIGTERM);
        free(hog_pids);
        return 1;
    }

    if (interactive_pid == 0) {
        char runtime_buf[32];
        snprintf(runtime_buf, sizeof(runtime_buf), "%d", runtime_seconds);
        char *child_argv[] = { (char *)"./interactive", runtime_buf, NULL };
        exec_or_die("./interactive", child_argv);
    }

    // 3) Wait for interactive to finish
    if (waitpid(interactive_pid, NULL, 0) < 0) {
        perror("waitpid(interactive)");
    }

    // 4) Terminate hogs
    for (int i = 0; i < num_hogs; i++) {
        if (kill(hog_pids[i], SIGTERM) < 0 && errno != ESRCH) {
            perror("kill(SIGTERM)");
        }
    }

    // Optional: small grace period, then SIGKILL stragglers
    usleep(100000); // 100ms
    for (int i = 0; i < num_hogs; i++) {
        if (kill(hog_pids[i], SIGKILL) < 0 && errno != ESRCH) {
            // ignore already-exited hogs
        }
    }

    // 5) Reap remaining children
    while (wait(NULL) > 0) {
        ;
    }

    free(hog_pids);
    return 0;
}