#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <types.h>

#define SYS_none             0
#define SYS_fork             1
#define SYS_exit             2
#define SYS_wait             3
#define SYS_pipe             4
#define SYS_read             5
#define SYS_kill             6
#define SYS_exec             7
#define SYS_fstat            8
#define SYS_chdir            9
#define SYS_dup             10
#define SYS_getpid          11
#define SYS_sbrk            12
#define SYS_sleep           13
#define SYS_uptime          14
#define SYS_open            15
#define SYS_write           16
#define SYS_mknod           17
#define SYS_unlink          18
#define SYS_link            19
#define SYS_mkdir           20
#define SYS_close           21
#define SYS_mount           22
#define SYS_umount          23
#define SYS_getppid         24
#define SYS_mmap            25
#define SYS_readdir         26
#define SYS_dup2            27
#define SYS_kdump           28
#define SYS_lseek           29
#define SYS_stat            30
#define SYS_lstat           31
#define SYS_signal          32
#define SYS_sigaction       33
#define SYS_sigpending      34
#define SYS_sigprocmask     35
#define SYS_sigsuspend      36
#define SYS_sigreturn       37
#define SYS_waitpid         38
#define SYS_fchdir          39
#define SYS_getcwd          40

#define NR_SYSCALL          41

#endif
