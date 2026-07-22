#!/bin/bash
# Log monitoring tool
# Watches a log file in real time, extracts ERROR lines to a report,
# and suppresses extra output using /dev/null

LOG_FILE="./system.log"
ERROR_REPORT="./error_report.txt"

touch "$LOG_FILE"
> "$ERROR_REPORT"

echo "Watching: $LOG_FILE"
echo "ERROR lines will be saved to: $ERROR_REPORT"
echo "Press Ctrl+C to stop"
echo ""

# The pipeline:
# tail -f streams new lines as they're added
# tee shows everything on screen and also passes to the sub-pipeline
# grep filters only ERROR lines
# tee -a appends those to the report file
# > /dev/null suppresses the duplicate output from the inner tee

tail -f "$LOG_FILE" \
    | tee >(grep --line-buffered "ERROR" | tee -a "$ERROR_REPORT" > /dev/null)
