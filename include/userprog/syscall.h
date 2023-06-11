#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
struct file_descriptor *find_file_descriptor (int fd);
#endif /* userprog/syscall.h */
