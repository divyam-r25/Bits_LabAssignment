# Question 2 - Process Management & Zombie Prevention

## What I did

I wrote a C program that spawns multiple child processes using fork(). The parent monitors each child, and if any child takes too long (over 4 seconds), it sends SIGTERM first and then SIGKILL if the child ignores it. A SIGCHLD handler with waitpid() makes sure no zombie processes are left behind.

## How to compile and run

```bash
gcc -o process_manager process_manager.c
./process_manager
```

---

## How each part works

**fork()**
```c
pid_t pid = fork();
```
fork() creates a copy of the current process. It returns the child's PID to the parent and 0 to the child. If it returns -1, something went wrong. This is how the web server creates worker processes to handle requests.

After fork(), we check the return value:
- pid == 0 → we're in the child, so run child_work()
- pid > 0 → we're in the parent, store the child's PID

**SIGCHLD + waitpid() - prevents zombie processes**
```c
struct sigaction sa;
sa.sa_handler = sigchld_handler;
sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
sigaction(SIGCHLD, &sa, NULL);
```
When a child process exits, the kernel sends SIGCHLD to the parent. Without a handler, the parent never reads the child's exit status, so the kernel keeps the process entry in the table - that's a zombie.

Inside the handler:
```c
while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { ... }
```
waitpid(-1, ...) means "any child". WNOHANG means "don't block if no child has exited yet". We loop because multiple children can exit between signals. This cleans up the zombie entries without pausing the parent.

**Monitoring loop + SIGTERM/SIGKILL**
```c
if (elapsed >= TIMEOUT_SEC) {
    kill(child_pids[i], SIGTERM);
    sleep(1);
    if (child_alive[i])
        kill(child_pids[i], SIGKILL);
}
```
The parent tracks how long each child has been running using time(). If elapsed time exceeds the timeout, it sends SIGTERM first - this gives the child a chance to clean up. If after 1 second it's still running, SIGKILL is sent which cannot be caught or ignored, so the process is guaranteed to die.

**WIFEXITED and WIFSIGNALED**
```c
if (WIFEXITED(status))
    printf("exited with code %d\n", WEXITSTATUS(status));
else if (WIFSIGNALED(status))
    printf("killed by signal %d\n", WTERMSIG(status));
```
These macros decode the status value returned by waitpid(). WIFEXITED checks if the child exited normally, WIFSIGNALED checks if it was killed by a signal.

---

## How everything works together

The SIGCHLD handler runs automatically when a child exits - it calls waitpid(WNOHANG) which removes the zombie entry from the process table and marks the child as inactive. Meanwhile the monitoring loop in main() checks for timeouts and sends signals to unresponsive processes. Together they ensure:
- Child processes that finish naturally are immediately reaped
- Child processes that hang are forcefully terminated
- No zombie processes remain after the program ends

---

## Sample output

```
Parent PID: 12345 | Timeout: 4s per child

[PARENT] spawned child 0, PID=12346, task=2s
[PARENT] spawned child 1, PID=12347, task=3s
[PARENT] spawned child 2, PID=12348, task=8s
[PARENT] spawned child 3, PID=12349, task=10s
[PARENT] spawned child 4, PID=12350, task=1s

Monitoring children...

[CHILD 4] PID 12350 - done
[PARENT] child 4 (PID 12350) exited with code 0
[CHILD 0] PID 12346 - done
[PARENT] child 0 (PID 12346) exited with code 0
[CHILD 1] PID 12347 - done
[PARENT] child 1 (PID 12347) exited with code 0
[MONITOR] child 2 (PID 12348): 4 sec elapsed
[MONITOR] child 2 is unresponsive - sending SIGTERM
[MONITOR] child 3 (PID 12349): 4 sec elapsed
[MONITOR] child 3 is unresponsive - sending SIGTERM
[PARENT] child 2 (PID 12348) killed by signal 15
[PARENT] child 3 (PID 12349) killed by signal 15

All children handled. No zombies left.
```
