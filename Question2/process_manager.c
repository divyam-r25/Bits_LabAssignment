/*
 * process_manager.c
 * Creates child processes, monitors them, kills unresponsive ones,
 * and prevents zombies using SIGCHLD handler with waitpid.
 *
 * Compile: gcc -o process_manager process_manager.c
 * Run:     ./process_manager
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define NUM_CHILDREN   5
#define TIMEOUT_SEC    4

pid_t  child_pids[NUM_CHILDREN];
time_t child_start[NUM_CHILDREN];
int    child_alive[NUM_CHILDREN];

/* SIGCHLD handler - called when any child exits.
   We use waitpid with WNOHANG so the parent doesn't block. */
void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < NUM_CHILDREN; i++) {
            if (child_pids[i] == pid) {
                child_alive[i] = 0;
                if (WIFEXITED(status))
                    printf("[PARENT] child %d (PID %d) exited with code %d\n",
                           i, pid, WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    printf("[PARENT] child %d (PID %d) killed by signal %d\n",
                           i, pid, WTERMSIG(status));
                break;
            }
        }
    }
    (void)sig;
}

void child_work(int id, int duration) {
    printf("[CHILD %d] PID %d - working for %d seconds\n", id, getpid(), duration);
    fflush(stdout);
    sleep(duration);
    printf("[CHILD %d] PID %d - done\n", id, getpid());
    fflush(stdout);
    exit(0);
}

int main(void) {
    /* tasks - child 2 and 3 will take longer than TIMEOUT_SEC */
    int durations[NUM_CHILDREN] = {2, 3, 8, 10, 1};

    printf("Parent PID: %d | Timeout: %ds per child\n\n", getpid(), TIMEOUT_SEC);

    /* register SIGCHLD handler before forking */
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    /* fork all children */
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            child_work(i, durations[i]);
        } else {
            child_pids[i]  = pid;
            child_start[i] = time(NULL);
            child_alive[i] = 1;
            printf("[PARENT] spawned child %d, PID=%d, task=%ds\n", i, pid, durations[i]);
        }
    }

    printf("\nMonitoring children...\n\n");

    int done = 0;
    while (!done) {
        sleep(1);
        done = 1;

        for (int i = 0; i < NUM_CHILDREN; i++) {
            if (!child_alive[i]) continue;
            done = 0;

            long elapsed = (long)(time(NULL) - child_start[i]);
            printf("[MONITOR] child %d (PID %d): %ld sec elapsed\n",
                   i, child_pids[i], elapsed);

            if (elapsed >= TIMEOUT_SEC) {
                printf("[MONITOR] child %d is unresponsive - sending SIGTERM\n", i);
                kill(child_pids[i], SIGTERM);
                sleep(1);

                if (child_alive[i]) {
                    printf("[MONITOR] child %d still alive - sending SIGKILL\n", i);
                    kill(child_pids[i], SIGKILL);
                }
            }
        }
    }

    printf("\nAll children handled. No zombies left.\n");
    return 0;
}
