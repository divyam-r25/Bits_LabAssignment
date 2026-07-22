/*
 * employee_records.c
 * Demonstrates file operations using Linux system calls:
 * open(), write(), lseek(), read(), close()
 * No standard library file functions (fopen, fprintf etc.) used.
 *
 * Compile: gcc -o employee_records employee_records.c
 * Run:     ./employee_records
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NAME_LEN  32
#define DEPT_LEN  16
#define DB_FILE   "employee.db"

typedef struct {
    int    emp_id;
    char   name[NAME_LEN];
    char   department[DEPT_LEN];
    double salary;
} Employee;

void show(const Employee *e) {
    printf("  ID: %d | Name: %-20s | Dept: %-12s | Salary: %.2f\n",
           e->emp_id, e->name, e->department, e->salary);
}

int main(void) {

    printf("=== Employee Records using Linux System Calls ===\n\n");

    /* 1. open() - create the file */
    int fd = open(DB_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); exit(1); }
    printf("[open] file '%s' opened, fd = %d\n", DB_FILE, fd);

    /* initial employee data */
    Employee emp[5] = {
        {101, "Alice Johnson",  "Engineering", 75000.00},
        {102, "Bob Smith",      "Marketing",   62000.00},
        {103, "Carol Williams", "Finance",      68000.00},
        {104, "David Brown",    "Engineering", 80000.00},
        {105, "Eva Martinez",   "HR",           58000.00}
    };

    /* 2. write() - write all records sequentially */
    printf("\n[write] Writing 5 records...\n");
    for (int i = 0; i < 5; i++) {
        ssize_t n = write(fd, &emp[i], sizeof(Employee));
        if (n == -1) { perror("write"); exit(1); }
        printf("  Written: ID=%d (%zd bytes)\n", emp[i].emp_id, n);
    }

    /* 3. lseek() + read() - read all records back */
    printf("\n[read] Reading all records:\n");
    for (int i = 0; i < 5; i++) {
        Employee e;
        lseek(fd, (off_t)i * sizeof(Employee), SEEK_SET);
        read(fd, &e, sizeof(Employee));
        show(&e);
    }

    /* 4. lseek() + write() - update record at index 1 (Bob - promoted) */
    Employee updated = {102, "Bob Smith", "Management", 90000.00};
    off_t pos = lseek(fd, 1 * sizeof(Employee), SEEK_SET);
    write(fd, &updated, sizeof(Employee));
    printf("\n[update] record 1 updated at byte offset %ld\n", (long)pos);

    /* verify the update */
    Employee check;
    lseek(fd, 1 * sizeof(Employee), SEEK_SET);
    read(fd, &check, sizeof(Employee));
    printf("[verify] record 1 now: ");
    show(&check);

    /* direct access - retrieve record at index 3 */
    Employee mid;
    lseek(fd, 3 * sizeof(Employee), SEEK_SET);
    read(fd, &mid, sizeof(Employee));
    printf("\n[direct access] record 3: ");
    show(&mid);

    /* 5. close() */
    close(fd);
    printf("\n[close] file descriptor %d closed\n", fd);
    printf("\nDB file: %s  (5 records x %zu bytes = %zu bytes total)\n",
           DB_FILE, sizeof(Employee), 5 * sizeof(Employee));

    return 0;
}
