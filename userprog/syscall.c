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
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/palloc.h"
#include "lib/kernel/stdio.h"
#include "userprog/process.h"


static struct file_descriptor *find_file_descriptor(int fd);
static int allocate_fd(void);

void halt(void);
void exit(int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);

void syscall_entry(void);
void syscall_handler(struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081			/* Segment selector msr */
#define MSR_LSTAR 0xc0000082		/* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void syscall_init(void)
{
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 |
							((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t)syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			  FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void syscall_handler(struct intr_frame *f UNUSED)
{
	// TODO: Your implementation goes here.
	// int *call_number;
	// call_number = f->R.rax;
	struct file_descriptor *file_desc;

	switch (f->R.rax)
	{ // rax : return value
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		exit(f->R.rdi); // R.rdi : 1st argument
		break;
	case SYS_FORK:
		f->R.rax = fork(f->R.rdi, f);
		break;
	case SYS_EXEC:
		f->R.rax = exec(f->R.rdi);
		break;
	// case SYS_WAIT:
	// 	wait();
	// 	break;
	case SYS_CREATE:
		f->R.rax = create(f->R.rdi, f->R.rsi); // R.rsi: 2nd argument
		break;
	case SYS_REMOVE:
		f->R.rax = remove(f->R.rdi);
		break;
	case SYS_OPEN:
		f->R.rax = open(f->R.rdi);
		break;
	case SYS_FILESIZE:
		f->R.rax = filesize(f->R.rdi);
		break;
	case SYS_READ:
		f->R.rax = read(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_WRITE:
		f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_SEEK:
		seek(f->R.rdi, f->R.rsi);
		break;
	case SYS_TELL:
		f->R.rax = tell(f->R.rdi);
		break;
	case SYS_CLOSE:
		close(f->R.rdi);
		break;
	default:
		thread_exit();
	}
}

void halt(void){
	power_off();
}

void exit(int status){
	// Terminates the current user program, returning status to the kernel.
	// If the process's parent waits for it (see below), this is the status that will be returned.
	// Conventionally, a status of 0 indicates success and nonzero values indicate errors.

	printf("%s:exit(%d)\n", thread_name(), status);
	thread_exit();
}

tid_t fork(const char *thread_name, struct intr_frame *f)
{
	// 	Create new process which is the clone of current process with the name THREAD_NAME.
	// 	You don't need to clone the value of the registers except %RBX, %RSP, %RBP, and %R12 - %R15, which are callee-saved registers.
	// 	Must return pid of the child process, otherwise shouldn't be a valid pid. In child process, the return value should be 0.
	// 	The child should have DUPLICATED resources including file descriptor and virtual memory space.
	// 	Parent process should never return from the fork until it knows whether the child process successfully cloned.
	// 	That is, if the child process fail to duplicate the resource, the fork () call of parent should return the TID_ERROR.

	// The template utilizes the pml4_for_each() in threads/mmu.c to copy entire user memory space, including corresponding pagetable structures,
	// but you need to fill missing parts of passed pte_for_each_func (See virtual address).

	process_fork(thread_name, f);
}

// 주어진 명령어 라인을 해석하여 인수 리스트를 생성
int exec(const char *cmd_line)
{
	return process_exec(cmd_line);
}

void check_address(void *addr)
{
	if (!is_user_vaddr(addr) || addr == NULL)
	{
		exit(-1);
	}
}

// // int wait(pid_t pid) {
// // 	/* waits for a child process pid and retrieves the child's exit status. if pid is still alive, waits unitl it terminates.
// // 	then, returns the status that pid passed to exit. if pid did not call exit(), but was terminated by the kernel(e.g. killed due to an exception),
// // 	wait(pid) must return -1. it tis perfectly legal for a parent process to wait for child processes that have already terminated by the time the parent calls wait,
// // 	but the kernel must still allow the parent to retrieve its child's exit status, or learn that the child was terminated by the kernel.

// // 	wiat must fail and return -1 immediately if any of the following conditions is true:
// // 	- pid does not refer to a direct child of the calling process. pid is a direct child of the calling process if and only if the calling process received pid as a return value
// // 	from a successful call to fork. note that children are not inherited: if a spawns child b and b spawns child process c,
// // 	then a cannot wait for c, even if b is dead. a call to wait(c) by process a must fail. similarly, orphaned processes are not assigned to a new parent if their parent process exists before they do.
// // 	- the process that calls wait has already called wait on pid. that is, a process may wait for any given child at most once.*/
// // };


bool create(const char *file, unsigned initial_size){
	if (file == NULL || file =="NULL") exit(-1);
	else return filesys_create(file, initial_size);
}

bool remove(const char *file){
	filesys_remove(file);
}


int open(const char *file)
{
	if (file = NULL) return -1;
	struct file *file_ = filesys_open(file);
	if (file_ == NULL) return -1;

	struct file_descriptor *file_desc = palloc_get_page(0);

	file_desc->fd = allocate_fd();
	file_desc->file = file_;

	list_push_back(&thread_current()->file_descriptors, &file_desc->elem);

	return file_desc->fd;
}



int filesize(int fd){
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	file_length(file_desc->file);
}

int read(int fd, void *buffer, unsigned size){
	if (!fd) return -1;
	if (fd == 0){
		unsigned i;
		for (i = 0; i < size; i++){
			*(uint8_t *)(&buffer + i) = input_getc();
		}
		return size;
	}

	struct file *file_ = find_file_descriptor(fd)->file;
	if (file_ == NULL) return -1;
	else
		return file_read(file_, buffer, size);
}

int write(int fd, const void *buffer, unsigned size)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);

	if (fd == 1){// fd = 1은 출력
		putbuf(buffer, size); // 버퍼에 있는 값을 SIZE만큼 출력
	} else{
		if (!file_desc){
			return -1;
		}
		return file_write(file_desc->file, buffer, size);
	}
}

void seek(int fd, unsigned position)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	file_seek(file_desc->file, position);
}

unsigned tell(int fd)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	return file_tell(file_desc->file);
}

void close(int fd){
	if (!fd || fd > 64) return -1;
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	if (file_desc != NULL){
		file_close(file_desc->file);
		list_remove(&file_desc->elem);
		palloc_free_page(file_desc);
	}
}

/*현재 스레드의 파일 디스크립터 리스트에서 주어진 파일 디스크립터(fd)와 일치하는 파일 디스크립터를 찾아서 반환*/
struct file_descriptor *find_file_descriptor(int fd)
{
	struct thread *cur = thread_current();
	struct list_elem *e;
	for (e = list_begin(&cur->file_descriptors); e != list_end(&cur->file_descriptors); e = list_next(e)){
		struct file_descriptor *file_desc = list_entry(e, struct file_descriptor, elem);
		if (file_desc->fd == fd) return file_desc;
	}
	return NULL;
}

int allocate_fd(void){
	struct thread *cur = thread_current();
	int fd = 2; // 0, 1은 표준 입력/출력이므로.
	while (true){
		if (find_file_descriptor(fd) == NULL) return fd;
		fd++; // 파일디스크립터 존재하면 fd++ 루프 반복
	}
}