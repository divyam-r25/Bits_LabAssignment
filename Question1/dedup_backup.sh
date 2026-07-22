#!/bin/bash

# Script to find duplicate submissions, back up unique ones, and generate a report
# All error messages go to a separate log file

SUBMISSIONS_DIR="./submissions"
BACKUP_DIR="./unique_backup"
REPORT_FILE="./report.txt"
ERROR_LOG="./errors.log"
HASH_FILE="./file_hashes.tmp"

mkdir -p "$SUBMISSIONS_DIR" "$BACKUP_DIR" 2>>"$ERROR_LOG"

# clear old output files
> "$REPORT_FILE"
> "$ERROR_LOG"
> "$HASH_FILE"

total=0
duplicates=0
backed_up=0

echo "Deduplication Report - $(date)" >> "$REPORT_FILE"
echo "==============================" >> "$REPORT_FILE"

for filepath in "$SUBMISSIONS_DIR"/*; do

    if [ ! -f "$filepath" ]; then
        echo "skipping: $filepath" >>"$ERROR_LOG"
        continue
    fi

    filename=$(basename "$filepath")
    total=$((total + 1))

    hash=$(md5sum "$filepath" 2>>"$ERROR_LOG" | awk '{print $1}')

    if [ -z "$hash" ]; then
        echo "ERROR: could not hash $filename" >>"$ERROR_LOG"
        continue
    fi

    if grep -q "^$hash " "$HASH_FILE" 2>>"$ERROR_LOG"; then
        duplicates=$((duplicates + 1))
        original=$(grep "^$hash " "$HASH_FILE" | awk '{print $2}')
        echo "DUPLICATE: $filename  (matches $original)" | tee -a "$REPORT_FILE"
    else
        echo "$hash $filename" >> "$HASH_FILE"
        cp "$filepath" "$BACKUP_DIR/$filename" 2>>"$ERROR_LOG"
        if [ $? -eq 0 ]; then
            backed_up=$((backed_up + 1))
            echo "BACKED UP: $filename" | tee -a "$REPORT_FILE"
        else
            echo "ERROR: backup failed for $filename" >>"$ERROR_LOG"
        fi
    fi

done

echo "" >> "$REPORT_FILE"
echo "Total files processed : $total" | tee -a "$REPORT_FILE"
echo "Duplicates found      : $duplicates" | tee -a "$REPORT_FILE"
echo "Files backed up       : $backed_up" | tee -a "$REPORT_FILE"
echo "Errors logged in      : $ERROR_LOG" | tee -a "$REPORT_FILE"

rm -f "$HASH_FILE"
