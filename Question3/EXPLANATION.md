# Question 3 - File I/O using Linux System Calls

## What I did

I wrote a C program that stores employee records in a binary file using Linux system calls - open(), write(), lseek(), read(), and close(). The key idea is using fixed-size structs so that any record can be accessed directly by calculating its byte offset, without reading the whole file.

## Compile and run

```bash
gcc -o employee_records employee_records.c
./employee_records
```

---

## System calls used and why

**open()**
```c
int fd = open("employee.db", O_RDWR | O_CREAT | O_TRUNC, 0644);
```
open() creates or opens the file and returns a file descriptor (an integer like 3 or 4) which we use in all subsequent operations. O_RDWR allows both reading and writing. O_CREAT creates it if missing, O_TRUNC empties it for a fresh start. 0644 is the permission mode (owner: read+write, others: read only).

This is the system call equivalent of fopen() but gives us direct kernel-level access without buffering.

**write()**
```c
ssize_t n = write(fd, &emp[i], sizeof(Employee));
```
write() sends raw bytes from memory to the file. Writing the entire struct at once creates a fixed-size record on disk. Every Employee struct takes exactly sizeof(Employee) bytes, which makes it possible to jump to any record directly.

**lseek()**
```c
lseek(fd, (off_t)i * sizeof(Employee), SEEK_SET);
```
lseek() moves the file's read/write position without any actual I/O. SEEK_SET means the offset is measured from the start of the file. To get record number i, the offset is simply i × sizeof(Employee). This gives O(1) random access - we can jump directly to any record without reading through all the previous ones.

This is the main advantage over text files where you'd have to scan from the beginning to find a specific entry.

**read()**
```c
read(fd, &e, sizeof(Employee));
```
After lseek() positions us at the right place, read() loads exactly sizeof(Employee) bytes into the struct. The binary data maps directly onto the struct fields, no parsing needed.

**Updating a record without rewriting the whole file**
```c
lseek(fd, 1 * sizeof(Employee), SEEK_SET);
write(fd, &updated, sizeof(Employee));
```
Since records are fixed size, updating record 1 just means seeking to byte position 1 × sizeof(Employee) and writing the new data. Only those bytes change - the rest of the file is untouched. This would be very important for a large database.

**close()**
```c
close(fd);
```
close() releases the file descriptor and flushes pending writes to disk. Not calling close() would cause a file descriptor leak - each process has a limited number (usually 1024), so leaking them in a long-running server would eventually cause failures.

---

## How open, read, write, lseek, close contribute to the solution

- open() → get a handle (fd) to work with
- write() → store fixed-size binary records sequentially
- lseek() → jump to any record's exact position in O(1) time
- read() → load that record into a struct directly
- close() → flush and release the handle

The fixed-size struct design is what makes lseek useful here. If records were variable length, you'd have no way to calculate where each one starts.

---

## Output

```
=== Employee Records using Linux System Calls ===

[open] file 'employee.db' opened, fd = 3

[write] Writing 5 records...
  Written: ID=101 (64 bytes)
  Written: ID=102 (64 bytes)
  Written: ID=103 (64 bytes)
  Written: ID=104 (64 bytes)
  Written: ID=105 (64 bytes)

[read] Reading all records:
  ID: 101 | Name: Alice Johnson          | Dept: Engineering   | Salary: 75000.00
  ID: 102 | Name: Bob Smith              | Dept: Marketing     | Salary: 62000.00
  ID: 103 | Name: Carol Williams         | Dept: Finance       | Salary: 68000.00
  ID: 104 | Name: David Brown            | Dept: Engineering   | Salary: 80000.00
  ID: 105 | Name: Eva Martinez           | Dept: HR            | Salary: 58000.00

[update] record 1 updated at byte offset 64
[verify] record 1 now:   ID: 102 | Name: Bob Smith    | Dept: Management | Salary: 90000.00

[direct access] record 3:   ID: 104 | Name: David Brown  | Dept: Engineering | Salary: 80000.00

[close] file descriptor 3 closed

DB file: employee.db  (5 records x 64 bytes = 320 bytes total)
```
