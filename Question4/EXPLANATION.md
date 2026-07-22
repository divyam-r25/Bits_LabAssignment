# Question 4 - Real-Time Log Monitoring Pipeline

## What I did

I designed a command pipeline that watches a log file in real time, shows all new entries on screen, and at the same time extracts only ERROR lines into a separate report file. /dev/null is used to suppress duplicate output.

## How to run

```bash
# Terminal 1 - start the monitor
chmod +x log_monitor.sh
./log_monitor.sh

# Terminal 2 - add entries to the log (to test it)
chmod +x simulate_log.sh
./simulate_log.sh
```

---

## The pipeline

```bash
tail -f system.log \
    | tee >(grep --line-buffered "ERROR" | tee -a error_report.txt > /dev/null)
```

---

## Each command and why it's there

**tail -f**
```bash
tail -f system.log
```
tail -f continuously monitors the file and prints each new line as it's added. Without the -f flag, tail would just show the last 10 lines and exit. -f keeps it open indefinitely, which is what we want for real-time monitoring. Internally it uses inotify to get notified by the kernel when new data is written, so it's not just polling in a loop.

**| (pipe)**
The pipe connects the output of tail -f to the input of tee. Data flows between processes in memory without touching the disk. The kernel handles the buffer between them. This is more efficient than saving intermediate output to a temp file.

**tee**
```bash
tee >(...)
```
tee reads from stdin and writes to two places at once - stdout (so everything shows on screen) and a file/process given as argument. Here the argument is a process substitution >(grep ...) which creates a sub-pipeline. So all log lines appear on terminal AND go into the grep filter simultaneously.

**grep --line-buffered "ERROR"**
```bash
grep --line-buffered "ERROR"
```
grep filters lines containing "ERROR". Normally grep buffers output internally which causes delays in a pipeline. The --line-buffered flag forces it to flush after each line, so ERROR entries appear in the report immediately when they arrive, not after some buffer fills up.

**tee -a error_report.txt**
```bash
tee -a error_report.txt > /dev/null
```
The inner tee appends ERROR lines to error_report.txt. The -a flag appends rather than overwriting. The > /dev/null after it discards the stdout of this inner tee - without this, ERROR lines would print twice on screen (once from outer tee, once from inner).

**/dev/null**
/dev/null is a special file that discards everything written to it. Writing to it is like throwing data away. It's used here to suppress the redundant terminal output from the inner tee. It's also useful in general to suppress error messages you don't care about, like: command 2>/dev/null

---

## How it all connects

```
system.log (new lines appended by web server)
    |
    v
tail -f          (streams each new line)
    |
    v
tee              (sends to both: terminal and the sub-pipeline)
  |          |
  v          v
terminal   grep "ERROR"        (filters ERROR lines only)
               |
               v
           tee -a error_report.txt   (saves to file, suppresses terminal copy)
               |
               v
           /dev/null
```

This design means:
- Admin sees ALL log entries live on screen
- Only ERROR entries get saved to the report
- No temp files, everything flows in memory through pipes
- No redundant output on the terminal

---

## Files

- log_monitor.sh - the monitoring script
- simulate_log.sh - helper to add fake entries for testing
- system.log - the log file being watched
- error_report.txt - gets populated with ERROR lines only

## error_report.txt contents after running

```
ERROR 2026-07-22 12:40:05 Connection timeout: client 192.168.1.15
ERROR 2026-07-22 12:40:08 Failed to write to disk: /var/log/app.log
ERROR 2026-07-22 12:40:10 Database query failed: timeout after 30s
ERROR 2026-07-22 12:40:13 Segmentation fault in worker process 4821
```
