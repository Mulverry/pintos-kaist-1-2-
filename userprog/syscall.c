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
	switch (f->R.rax)
	{
<<<<<<< HEAD
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		exit(f->R.rdi);
		break;
	// case SYS_FORK:
	// 	break;
	case SYS_EXEC:
		f->R.rax = exec(f->R.rdi);
		break;
	// case SYS_WAIT:
	// 	break;
	case SYS_CREATE:
		f->R.rax = create(f->R.rdi, f->R.rsi);
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

int allocate_fd(void)
{
	struct thread *cur = thread_current();
	int fd = 2;
	while (true)
	{
		if (find_file_descriptor(fd) == NULL)
			return fd;
		fd++;
=======
		// case SYS_HALT:
		// 	break;
		// case SYS_EXIT:
		// 	break;
		// case SYS_FORK:
		// 	break;
		// case SYS_EXEC:
		// 	break;
		// case SYS_WAIT:
		// 	break;
		// case SYS_CREATE:
		// 	break;
		// case SYS_REMOVE:
		// 	break;
		// case SYS_OPEN:
		// 	break;
		// case SYS_FILESIZE:
		// 	break;
		// case SYS_READ:
		// 	break;
		// case SYS_WRITE:
		// 	break;
		// case SYS_SEEK:
		// 	break;
		// case SYS_TELL:
		// 	break;
		// case SYS_CLOSE:
		// 	break;
>>>>>>> 80eda32e1331af6d3b043bf5e5c67eedab74c12f
	}
}

struct file_descriptor *find_file_descriptor(int fd)
{
	struct thread *cur = thread_current();
	struct list_elem *e;
	for (e = list_begin(&cur->file_descriptors); e != list_end(&cur->file_descriptors); e = list_next(e))
	{
		struct file_descriptor *file_desc = list_entry(e, struct file_descriptor, elem);
		if (file_desc->fd == fd)
			return file_desc;
	}
	return NULL;
}

void halt(void)
{
	power_off();
}

void exit(int status)
{
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_exit();
}

/* pid_t fork(const char *thread_name)
{
	/////////////////////////////////////////////
} */

int exec(const char *cmd_line)
{
	return process_exec(cmd_line);
}

/* int wait(pid_t pid)
{
	/////////////////////////////////////////////
} */

/* file(첫 번째 인자)를 이름으로 하고 크기가 initial_size(두 번째 인자)인 새로운 파일을 생성 */
bool create(const char *file, unsigned initial_size)
{
	if (file == "NULL" | file == NULL)
		exit(-1);
	else
		return filesys_create(file, initial_size);
}

/* file(첫 번째)라는 이름을 가진 파일을 삭제 */
bool remove(const char *file)
{
	if (file == "NULL" | file == NULL)
		exit(-1);
	else
		return filesys_remove(file);
}

int open(const char *file)
{
	if (file == NULL)
		return -1;

	struct file *file_ = filesys_open(file);
	if (file_ == NULL)
		return -1;

	struct file_descriptor *file_desc = palloc_get_page(0);

	file_desc->fd = allocate_fd();
	file_desc->file = file_;
	list_push_back(&thread_current()->file_descriptors, &file_desc->elem);

	if (file_desc->fd >= 0)
		return file_desc->fd;
	else
		return -1;
}

int filesize(int fd)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	file_length(file_desc->file);
}

int read(int fd, void *buffer, unsigned size)
{
	/*
	 * 1. FD 번호로 열려있는 파일로부터 SIZE bytes 만큼 읽어 buffer로 넣기
	 * 2. 읽은 bytes 만큼 반환
	 */
	if (!fd)
		return -1;
	if (fd == 0)
	{
		unsigned i;
		for (i = 0; i < size; i++)
		{
			*(uint8_t *)(buffer + i) = input_getc();
		}
		return size;
	}

	struct file_descriptor *file_ = find_file_descriptor(fd);

	if (file_ == NULL)
		return -1;
	else
		return file_read(file_->file, buffer, size);
}

int write(int fd, const void *buffer, unsigned size)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);

	if (fd == 1)
	{
		putbuf(buffer, size);
	}
	else
	{
		if (!file_desc)
			return -1;
		else
			return file_write(file_desc->file, buffer, size);
	}
}

void seek(int fd, unsigned position)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	file_seek(file_desc->file, position);
}

/* 열려진 파일 fd에서 읽히거나 써질 다음 바이트의 위치를 반환합니다. 파일의 시작지점부터 몇바이트인지로 표현 */
unsigned tell(int fd)
{
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	return file_tell(file_desc->file);
}

void close(int fd)
{
	if (!fd || fd > 64)
		return -1;
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	if (file_desc != NULL)
	{
		file_close(file_desc->file);
		list_remove(&file_desc->elem);
		palloc_free_page(file_desc);
	}
}

////
