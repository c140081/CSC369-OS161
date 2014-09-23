/* BEGIN A3 SETUP */
/*
 * Declarations for file handle and file table management.
 * New for A3.
 */

#ifndef _FILE_H_
#define _FILE_H_

#include <kern/limits.h>
#include <spinlock.h>

struct vnode;

/* File table Entries */
struct filetable_entry {
	struct vnode *ft_vnode; /* vnode for this file */

	struct lock *ft_lock;
	int ft_pos; /* position in file */
	int ft_flags; /* flags */
	int ft_count; /* count of number of fd pointing to this entry */
};


/*
 * filetable struct
 * just an array, nice and simple.  
 * It is up to you to design what goes into the array.  The current
 * array of ints is just intended to make the compiler happy.
 */
struct filetable {
	struct filetable_entry *ft_entries[__OPEN_MAX]; /* entries of this file table */
};

/* these all have an implicit arg of the curthread's filetable */
int filetable_init(void);
void filetable_destroy(struct filetable *ft);

/* opens a file (must be kernel pointers in the args) */
int file_open(char *filename, int flags, int mode, int *retfd);

/* closes a file */
int file_close(int fd);

/* A3: You should add additional functions that operate on
 * the filetable to help implement some of the filetable-related
 * system calls.
 */
/* search a file */
int filetable_searchfile(struct filetable_entry **fe, int fd);

#endif /* _FILE_H_ */

/* END A3 SETUP */
