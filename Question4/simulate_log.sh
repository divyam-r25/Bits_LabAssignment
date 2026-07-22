#!/bin/bash
# ============================================================
# simulate_log.sh – Appends realistic log entries to system.log
# Run this alongside log_monitor.sh to see real-time monitoring
# ============================================================

LOG_FILE="./system.log"

log_entries=(
    "INFO  2026-07-22 12:40:01 Web server started on port 8080"
    "INFO  2026-07-22 12:40:02 Database connection established"
    "DEBUG 2026-07-22 12:40:03 Request received from 192.168.1.10"
    "INFO  2026-07-22 12:40:04 GET /api/users HTTP/1.1 200 OK"
    "ERROR 2026-07-22 12:40:05 Connection timeout: client 192.168.1.15"
    "INFO  2026-07-22 12:40:06 GET /api/products HTTP/1.1 200 OK"
    "WARNING 2026-07-22 12:40:07 High memory usage: 87%"
    "ERROR 2026-07-22 12:40:08 Failed to write to disk: /var/log/app.log"
    "INFO  2026-07-22 12:40:09 Scheduled task completed successfully"
    "ERROR 2026-07-22 12:40:10 Database query failed: timeout after 30s"
    "DEBUG 2026-07-22 12:40:11 Cache miss for key: user_session_4892"
    "INFO  2026-07-22 12:40:12 POST /api/login HTTP/1.1 200 OK"
    "ERROR 2026-07-22 12:40:13 Segmentation fault in worker process 4821"
    "INFO  2026-07-22 12:40:14 Worker process restarted"
    "INFO  2026-07-22 12:40:15 All systems nominal"
)

echo "Simulating log entries for $LOG_FILE..."

for entry in "${log_entries[@]}"; do
    echo "$entry" >> "$LOG_FILE"
    echo "[WRITTEN] $entry"
    sleep 0.5   # Add entries every 0.5 seconds to simulate real-time
done

echo ""
echo "Log simulation complete. Check $LOG_FILE and error_report.txt"
