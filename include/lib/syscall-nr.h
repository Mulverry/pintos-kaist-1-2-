#ifndef __LIB_SYSCALL_NR_H
#define __LIB_SYSCALL_NR_H

/* System call numbers. */
enum
{
	/* Projects 2 and later. */
	SYS_HALT,	  /* 0 Halt the operating system. */
	SYS_EXIT,	  /* 1 Terminate this process. */
	SYS_FORK,	  /* 2 Clone current process. */
	SYS_EXEC,	  /* 3 Switch current process. */
	SYS_WAIT,	  /* 4 Wait for a child process to die. */
	SYS_CREATE,	  /* 5 Create a file. */
	SYS_REMOVE,	  /* 6 Delete a file. */
	SYS_OPEN,	  /* 7 Open a file. */
	SYS_FILESIZE, /* 8 Obtain a file's size. */
	SYS_READ,	  /* 9 Read from a file. */
	SYS_WRITE,	  /* 10 Write to a file. */
	SYS_SEEK,	  /* 11 Change position in a file. */
	SYS_TELL,	  /* 12 Report current position in a file. */
	SYS_CLOSE,	  /* 13 Close a file. */

	/* Project 3 and optionally project 4. */
	SYS_MMAP,	/* Map a file into memory. */
	SYS_MUNMAP, /* Remove a memory mapping. */

	/* Project 4 only. */
	SYS_CHDIR,	 /* Change the current directory. */
	SYS_MKDIR,	 /* Create a directory. */
	SYS_READDIR, /* Reads a directory entry. */
	SYS_ISDIR,	 /* Tests if a fd represents a directory. */
	SYS_INUMBER, /* Returns the inode number for a fd. */
	SYS_SYMLINK, /* Returns the inode number for a fd. */

	/* Extra for Project 2 */
	SYS_DUP2, /* Duplicate the file descriptor */

	SYS_MOUNT,
	SYS_UMOUNT,
};

#endif /* lib/syscall-nr.h */
