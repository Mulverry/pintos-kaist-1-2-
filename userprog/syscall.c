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
#include "threads/vaddr.h"
// #include "lib/user/syscall.h"


typedef int pid_t;
struct file_descriptor *find_file_descriptor (int fd);
int allocate_fd (void);

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

void check_address(void *addr);

void halt (void) NO_RETURN;
void exit (int status) NO_RETURN;

int exec (const char *file);
int wait (pid_t);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
pid_t fork (const char *thread_name, struct intr_frame *if_);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
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
	//TODO: Your implementation goes here.

	switch (f->R.rax) {
		case SYS_HALT:
			halt();
			break;
		case SYS_EXIT:
			exit(f->R.rdi);
			break;
		case SYS_FORK:
			memcpy(&thread_current()->parent_if, f, sizeof(struct intr_frame));
			f->R.rax = fork(f->R.rdi, &thread_current()->parent_if);
			break;
		case SYS_EXEC:
			f->R.rax = exec(f->R.rdi);
			break;
		case SYS_WAIT:
			f->R.rax = wait(f->R.rdi);
			break;
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

void check_address(void *addr){
	struct thread *current = thread_current();
	if (!is_user_vaddr(addr) || addr == NULL || pml4_get_page(current->pml4, addr)) {
		exit(-1);
	}
}

void halt (void) {
	power_off();
}

void exit (int status) {
	struct thread *cur = thread_current();
	cur->exit_status = status;
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_exit();
}

// 피호출자(callee) 저장 레지스터인 %RBX, %RSP, %RBP와 %R12 - %R15를 제외한 레지스터 값을 복제할 필요가 없습니다. 
// 자식 프로세스의 pid를 반환해야 합니다. 그렇지 않으면 유효한 pid가 아닐 수 있습니다. 자식 프로세스에서 반환 값은 0이어야 합니다. 
// 자식 프로세스에는 파일 식별자 및 가상 메모리 공간을 포함한 복제된 리소스가 있어야 합니다. 
// 부모 프로세스는 자식 프로세스가 성공적으로 복제되었는지 여부를 알 때까지 fork에서 반환해서는 안 됩니다. 
// 즉, 자식 프로세스가 리소스를 복제하지 못하면 부모의 fork() 호출이 TID_ERROR를 반환할 것입니다.
// 템플릿은 `threads/mmu.c`의 `pml4_for_each`를 사용하여 해당되는 페이지 테이블 구조를 포함한 전체 사용자 메모리 공간을 복사하지만, 
// 전달된 `pte_for_each_func`의 누락된 부분을 채워야 합니다.
pid_t fork (const char *thread_name, struct intr_frame *if_){

	return process_fork(thread_name, if_);
}

int exec (const char *cmd_line) {
	if ((is_user_vaddr(cmd_line) == false) || (cmd_line == NULL) || (pml4_get_page (thread_current()->pml4, cmd_line) == NULL))
		exit(-1);
	return process_exec(cmd_line);
	// check_address(cmd_line);
	// char *fn_copy = palloc_get_page(PAL_ZERO);

	// if (fn_copy == NULL) return -1;
	// strlcpy(fn_copy, cmd_line, PGSIZE);
	
	// if (process_exec(fn_copy) == -1) return -1;

	// NOT_REACHED();
	// return 0;
}


// 자식 프로세스 (pid) 를 기다려서 자식의 종료 상태(exit status)를 가져옵니다. 만약 pid (자식 프로세스)가 아직 살아있으면, 
// 종료 될 때 까지 기다립니다. 종료가 되면 그 프로세스가 exit 함수로 전달해준 상태(exit status)를 반환합니다. 

// 만약 pid (자식 프로세스)가 exit() 함수를 호출하지 않고 커널에 의해서 종료된다면 
// (e.g exception에 의해서 죽는 경우), wait(pid) 는  -1을 반환해야 합니다. 

// 부모 프로세스가 wait 함수를 호출한 시점에서 이미 종료되어버린 자식 프로세스를 기다리도록 하는 것은 완전히 합당합니다만, 
// 커널은 부모 프로세스에게 자식의 종료 상태를 알려주든지, 커널에 의해 종료되었다는 사실을 알려주든지 해야 합니다.
// 다음의 조건들 중 하나라도 참이면 wait 은 즉시 fail 하고 -1 을 반환합니다 :
// pid 는 호출하는 프로세스의 직속 자식을 참조하지 않습니다. 오직 호출하는 프로세스가 fork() 호출 후 성공적으로 pid를 반환받은 경우에만, 
// pid 는 호출하는 프로세스의 직속 자식입니다.

// 자식들은 상속되지 않는다는 점을 알아두세요 :  만약 A 가 자식 B를 낳고 B가 자식 프로세스 C를  낳는다면, A는 C를 기다릴 수 없습니다. 
// 심지어 B가 죽은 경우에도요. 프로세스 A가  wait(C) 호출하는 것은 실패해야 합니다. 

// 마찬가지로, 부모 프로세스가 먼저 종료되버리는 고아 프로세스들도 새로운 부모에게 할당되지 않습니다.
// wait 를 호출한 프로세스가 이미 pid에 대해 기다리는 wait 를 호출한 상태 일 때 입니다. 
// 즉, 한 프로세스는 어떤 주어진 자식에 대해서 최대 한번만 wait 할 수 있습니다.
// 프로세스들은 자식을 얼마든지 낳을 수 있고 그 자식들을 어떤 순서로도 기다릴 (wait) 수 있습니다. 
// 자식 몇 개로부터의 신호는 기다리지 않고도 종료될 수 있습니다. (전부를 기다리지 않기도 합니다.) 

// 여러분의 설계는 발생할 수 있는 기다림의 모든 경우를 고려해야합니다. 
// 한 프로세스의 (그 프로세스의 struct thread 를 포함한) 자원들은 꼭 할당 해제되어야 합니다. 

// 부모가 그 프로세스를 기다리든 아니든, 자식이 부모보다 먼저 종료되든 나중에 종료되든 상관없이 이뤄져야  합니다.
// 최초의 process가 종료되기 전에 Pintos가 종료되지 않도록 하십시오. 

// 제공된 Pintos 코드는 main() (in threads/init.c)에서 process_wait() (in userprog/process.c ) 를 
// 호출하여 Pintos가 최초의 process 보다 먼저 종료되는 것을 막으려고 시도합니다.  

// 여러분은 함수 설명의 제일 위의 코멘트를 따라서 process_wait() 를 구현하고 process_wait() 의 방식으로 
// wait system call을 구현해야 할 겁니다.
// 이 시스템 콜을 구현하는 것이 다른 어떤 시스템콜을 구현하는 것보다 더 많은 작업을 요구합니다.

int wait(pid_t pid) {
	return process_wait(pid);
};



bool create (const char *file, unsigned initial_size) {
	// check_address(file);
	// if (file == "NULL" | file == NULL) exit(-1);
	if ((is_user_vaddr(file) == false) || (file == NULL) || (pml4_get_page (thread_current()->pml4, file) == NULL))
		exit(-1);
	if (file == NULL) exit(-1);
	else	return filesys_create(file, initial_size);
}

bool remove (const char *file) {
	if ((is_user_vaddr(file) == false) || (file == NULL) || (pml4_get_page (thread_current()->pml4, file) == NULL))
		exit(-1);
	return filesys_remove(file);
}

int open (const char *file) {
	if ((is_user_vaddr(file) == false) || (file == NULL) || (pml4_get_page (thread_current()->pml4, file) == NULL))
		exit(-1);
	if (file == NULL) return -1;
	
	struct file *file_ = filesys_open(file);
	if (file_ == NULL) return -1;  // 파일이 생성되지 않으면 fail

	// struct file **fdt = thread_current()->file_descriptors_table;
	// int fd = thread_current()->fdidx;

	// while (thread_current()->file_descriptors_table[fd] != NULL && fd < 64) {
	// 	fd++;
	// }
	
	struct file_descriptor *file_desc = palloc_get_page(0);

	file_desc->fd = allocate_fd();
	file_desc->file = file_;


	list_push_back(&thread_current()->file_descriptors, &file_desc->elem);
	
	if (file_desc->fd >= 0) return file_desc->fd;
	else return -1;
}

int filesize (int fd) {
	struct file_descriptor *file_desc = find_file_descriptor(fd);
	file_length(file_desc->file);

	// struct file **fdt = thread_current()->file_descriptors_table;
	// struct file *file = fdt[fd]; 
	
	// if (file == NULL) return -1;

	// file_length(file);
}
	

int read (int fd, void *buffer, unsigned size) {
	check_address(buffer);
	if (!fd || fd == 1) return -1;
	if (fd == 0) {
		unsigned i;
		for (i = 0; i < size; i++)
		{
			*(uint8_t *)(&buffer + i) = input_getc();
		}
		return size;
	}

	// struct file_descriptor *file_ = find_file_descriptor(fd);
	//if (file_ == NULL)	return -1;
	//else return file_read(file_->file, buffer, size);
	
	struct file **fdt = thread_current()->file_descriptors_table;
	struct file *file = fdt[fd]; 

	if (file == NULL) return -1;
	else {
		lock_acquire(&file_lock);
		file_read(file, buffer, size);
		lock_release(&file_lock);
	}

	return file_read(file, buffer, size);
}

int write (int fd, const void *buffer, unsigned size) {
	check_address(buffer);

	struct file_descriptor *file_desc = find_file_descriptor(fd);
	// struct file **fdt = thread_current()->file_descriptors_table;
	// struct file *file = fdt[fd];
	// int read_count;

	struct file_descriptor *file_desc = find_file_descriptor(fd);
	
	if (fd == 1) {
	lock_acquire(&file_lock);

	if (fd == 0) { // 입력
		putbuf(buffer, size);
		return read_count = size;
	} else if (fd == 1) {// 출력
		return -1;
	} 
	else if (fd >= 2) { 
		if (!file) {
			lock_release(&file_lock);
			return -1;
		}
		read_count = file_write(file, buffer, size);
	}
	lock_release(&file_lock);
	return read_count;
}


void seek (int fd, unsigned position) {
	// struct file_descriptor *file_desc = find_file_descriptor(fd);
	// file_seek(file_desc->file, position);

	// struct file **fdt = thread_current()->file_descriptors_table;
	// struct file *file = fdt[fd];

	// file_seek(file, position);
}


unsigned tell (int fd) {

	struct file_descriptor *file_desc = find_file_descriptor(fd);
	return file_tell(file_desc->file);

	// struct file **fdt = thread_current()->file_descriptors_table;
	// struct file *file = fdt[fd]; 

	// return file_tell(file);
}


void close (int fd) {
	struct file_descriptor *file_desc = find_file_descriptor(fd);
    if (file_desc != NULL) {
        file_close(file_desc->file);
        list_remove(&file_desc->elem);
        palloc_free_page(file_desc);
    }

	// struct file **fdt = thread_current()->file_descriptors_table;
	// struct file *file = fdt[fd]; 

	// if (file == NULL) return;
	// if (fd < 2 || fd >=64) return;

	// fdt[fd] = NULL;

}


// struct file_descriptor *find_file_descriptor (int fd) {
// 	struct thread *cur = thread_current();
// 	struct list_elem *e;
// 	for (e = list_begin(&cur->file_descriptors_table); e != list_end(&cur->file_descriptors_table); e = list_next(e)) {
// 		struct file_descriptor *file_desc = list_entry(e, struct file_descriptor, elem);
// 		if (file_desc->fd == fd) {
// 			return file_desc;
// 			}
// 	}
// 	return NULL;
// }

// struct file *find_file_descriptor(int fd){
// 	struct thread *current = thread_current();

// 	for (int i= 2; i< 64; i++){
// 		struct file *file = parent->file_descriptors_table[i];
// 		if (file == NULL) continue;
// 		current->file_descriptors_table[i] = file_duplicate(i);
// 	}

// 	return NULL;
// }

// int allocate_fd (void) {
// 	struct thread *cur = thread_current();
// 	int fd = 2;
// 	while (true) {
// 		if (find_file_descriptor(fd) == NULL) return fd;
// 		fd++; // 파일디스크립터 존재하면 fd++ 루프 반복
// 	}
// }