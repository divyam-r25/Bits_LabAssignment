# Question 1 - Duplicate Submission Detector

## What I did

I wrote a shell script that goes through a submissions folder, computes an MD5 hash for each file, and checks if that hash already appeared before. If yes, it's a duplicate. If no, it copies the file to a backup folder.

A final report is written to report.txt and any errors (like unreadable files) go to errors.log separately.

## How to run

```bash
chmod +x dedup_backup.sh
./dedup_backup.sh
```

## Files

- dedup_backup.sh - main script
- submissions/ - folder with student files (I put 5 sample files, 2 of which are duplicates)
- report.txt - output report
- errors.log - any error messages

---

## Commands used and why

**mkdir -p**
```bash
mkdir -p "$SUBMISSIONS_DIR" "$BACKUP_DIR" 2>>"$ERROR_LOG"
```
Creates the required directories. The -p flag means it won't fail if they already exist. The 2>> sends any errors to errors.log instead of showing them on screen.

**> (truncate redirect)**
```bash
> "$REPORT_FILE"
```
This clears the file before starting so the report is fresh each time. Using > without a command just empties the file.

**md5sum**
```bash
hash=$(md5sum "$filepath" | awk '{print $1}')
```
md5sum gives a unique fingerprint based on file content, not the filename. So even if a student renames their copy, it still gets caught. awk '{print $1}' just extracts the hash part.

**grep -q**
```bash
if grep -q "^$hash " "$HASH_FILE"; then
```
The -q flag makes grep silent - we only care about the exit code (0 = found, 1 = not found). The ^ anchors the match to the start of the line so partial matches don't happen.

**tee -a**
```bash
echo "BACKED UP: $filename" | tee -a "$REPORT_FILE"
```
tee writes to both the terminal and the file at the same time. The -a means append, so nothing gets overwritten.

**2>> (stderr redirect)**
```bash
cp "$filepath" "$BACKUP_DIR/$filename" 2>>"$ERROR_LOG"
```
2>> sends only error messages to errors.log. Normal output (fd 1) still goes to terminal. This keeps error output separate from the report.

---

## Output

```
BACKED UP: alice_assignment.txt
BACKED UP: bob_assignment.txt
DUPLICATE: charlie_assignment.txt  (matches alice_assignment.txt)
BACKED UP: diana_assignment.txt
DUPLICATE: eve_assignment.txt  (matches bob_assignment.txt)

Total files processed : 5
Duplicates found      : 2
Files backed up       : 3
Errors logged in      : ./errors.log
```

---

## Why each technique was used

The reason I used md5sum instead of comparing filenames is that students can easily rename files. Content-based hashing is more reliable. 

I kept errors separate in errors.log using 2>> because mixing error messages with the report would make it hard to read. 

tee is useful because the professor/admin running the script can see results live on screen while they're also being saved to the file at the same time.

The temporary hash file (file_hashes.tmp) is deleted at the end with rm -f to keep the folder clean.
