#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/init.h"
#include <string.h>


void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}



/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	int *call_number;
	call_number = f->R.rax;
	switch (*call_number) {
		case SYS_HALT:
			halt();
			break;
	// 	case SYS_EXIT:
	// 		exit();
			// break;
	// 	case SYS_FORK:
	// 		fork(thread_name());
			// break;
	// 	case SYS_EXEC:
	// 		exec();
			// break;
	// 	case SYS_WAIT:
	// 		wait();
			// break;
			/// 연주

	// 	case SYS_CREATE:
	// 		create();
			// break;
	// 	case SYS_REMOVE:
	// 		remove();
			// break;
	// 	case SYS_OPEN:
	// 		open();
			// break;
	// 	case SYS_FILESIZE:
	// 		filesize();
			// break;
			// 승규

	// 	case SYS_READ:
	// 		read();
			// break;
	// 	case SYS_WRITE:
	// 		write();
			// break;
	// 	case SYS_SEEK:
	// 		seek();
			// break;
	// 	case SYS_TELL:
	// 		tell();
			// break;
	// 	case SYS_CLOSE:
	// 		close();
			// break;
			// 희령
	}


	printf ("system call!\n");
	thread_exit ();
}

void halt (void) {
	power_off();
}

// void exit (int status) {
// 	// Terminates the current user program, returning status to the kernel. 
// 	// If the process's parent waits for it (see below), this is the status that will be returned. 
// 	// Conventionally, a status of 0 indicates success and nonzero values indicate errors.
// 	printf("%s: exit(%s)", process_name, status);
// 	thread_exit();
		
// 	return 0;
// }

// pid_t fork (const char *thread_name){
// // 	Create new process which is the clone of current process with the name THREAD_NAME. 
// // 	You don't need to clone the value of the registers except %RBX, %RSP, %RBP, and %R12 - %R15, which are callee-saved registers. 
// // 	Must return pid of the child process, otherwise shouldn't be a valid pid. In child process, the return value should be 0. 
// // 	The child should have DUPLICATED resources including file descriptor and virtual memory space. 
// // 	Parent process should never return from the fork until it knows whether the child process successfully cloned. 
// // 	That is, if the child process fail to duplicate the resource, the fork () call of parent should return the TID_ERROR.

// // The template utilizes the pml4_for_each() in threads/mmu.c to copy entire user memory space, including corresponding pagetable structures, 
// // but you need to fill missing parts of passed pte_for_each_func (See virtual address).
// }

// int exec (const char *cmd_line) {
// 	struct list args_list;
// 	list_init(&args_list);
	
// 	char *ptr;
// 	char *line = strtok_r(cmd_line, " ", &ptr);
// 	while (line != NULL) {
// 		struct list_item *new_item = malloc(sizeof(struct list_item));
// 		new_item->str = line;
// 		list_push_back(&args_list, &new_item->elem);
// 		line = strtok_r(NULL, " ", &ptr);
// 	}

	
// }



// int wait(pid_t pid) {
// 	/* waits for a child process pid and retrieves the child's exit status. if pid is still alive, waits unitl it terminates.
// 	then, returns the status that pid passed to exit. if pid did not call exit(), but was terminated by the kernel(e.g. killed due to an exception),
// 	wait(pid) must return -1. it tis perfectly legal for a parent process to wait for child processes that have already terminated by the time the parent calls wait, 
// 	but the kernel must still allow the parent to retrieve its child's exit status, or learn that the child was terminated by the kernel.
	
// 	wiat must fail and return -1 immediately if any of the following conditions is true:
// 	- pid does not refer to a direct child of the calling process. pid is a direct child of the calling process if and only if the calling process received pid as a return value
// 	from a successful call to fork. note that children are not inherited: if a spawns child b and b spawns child process c,
// 	then a cannot wait for c, even if b is dead. a call to wait(c) by process a must fail. similarly, orphaned processes are not assigned to a new parent if their parent process exists before they do.
// 	- the process that calls wait has already called wait on pid. that is, a process may wait for any given child at most once.*/
// };
bool create (const char *file, unsigned initial_size) {

};
bool remove (const char *file) {

};
int open (const char *file) {

};
int filesize (int fd) {

};
int read (int fd, void *buffer, unsigned size) {

};
int write (int fd, const void *buffer, unsigned size) {

};
void seek (int fd, unsigned position) {

};
unsigned tell (int fd) {

};
void close (int fd) {
	
};
