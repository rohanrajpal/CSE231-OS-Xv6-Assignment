//
// Created by rohan on 28/3/19.
//

#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "vm.h"

//char buf[8192];
//char name[3];
//char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
//int stdout = 1;
//#define TOTAL_MEMORY (2 << 20) + (1 << 18) + (1 << 17)

void
mem(void)
{
    struct proc *curproc = myproc();
    select_a_victim(curproc->pgdir);

    exit();
}

int
main(int argc, char *argv[])
{
    printf(1, "memtest starting\n");
    mem();
    return 0;
}