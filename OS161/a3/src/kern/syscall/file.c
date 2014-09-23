/* BEGIN A3 SETUP */
/*
 * File handles and file tables.
 * New for ASST3
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/unistd.h>
#include <file.h>
#include <syscall.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <synch.h>
#include <current.h>

/*** openfile functions ***/

/*
 * file_open
 * opens a file, places it in the filetable, sets RETFD to the file
 * descriptor. the pointer arguments must be kernel pointers.
 * NOTE -- the passed in filename must be a mutable string.
 * 
 * A3: As per the OS/161 man page for open(), you do not need 
 * to do anything with the "mode" argument.
 */
int
file_open(char *filename, int flags, int mode, int *retfd)
{	
	struct vnode *vn;
	int result;

	/* Ensure that filename is not null */
	if (filename == NULL){
		return EINVAL;
	}

	/* Check if file is already open */
	result = vfs_open(filename, flags, mode, &vn);
	if (result){
		return result;
	}
	
	/* OS161 manual for open() says that flag has to be one of the
	* O_RDONLY, O_WRONGLY or O_RDWR
	*/
	
	int maskedFlag = flags & O_ACCMODE;
	if (maskedFlag != O_RDONLY && maskedFlag != O_WRONLY && maskedFlag != O_RDWR){
		return EINVAL;
	}

	/* Find an empty (NULL) entry in filetable */
	int fd;

	for (fd = 0; fd < __OPEN_MAX; fd++){
		if (curthread->t_filetable->ft_entries[fd] == NULL){
			break;
		}
	}
	
	/* Check if file table is full */
	if (fd == __OPEN_MAX){
		return EMFILE;
	}

	/* Reserve memory for file */
	curthread->t_filetable->ft_entries[fd] = (struct filetable_entry *)kmalloc(sizeof(struct filetable_entry));
	if (curthread->t_filetable->ft_entries[fd] == NULL){
		vfs_close(vn);
		return ENOMEM;
	}

	/* initilize the entries */
	curthread->t_filetable->ft_entries[fd]->ft_lock = lock_create("file entry lock");

	curthread->t_filetable->ft_entries[fd]->ft_vnode = vn;
	curthread->t_filetable->ft_entries[fd]->ft_pos = 0;
	curthread->t_filetable->ft_entries[fd]->ft_flags = flags;
	curthread->t_filetable->ft_entries[fd]->ft_count = 1;

	*retfd = fd;

	return 0;
}


/* 
 * file_close
 * Called when a process closes a file descriptor.  Think about how you plan
 * to handle fork, and what (if anything) is shared between parent/child after
 * fork.  Your design decisions will affect what you should do for close.
 */
int
file_close(int fd)
{
	/* if the file descriptor is invalid, return error */
	if ((fd < 0) || (fd >= __OPEN_MAX) || (curthread->t_filetable->ft_entries[fd] == NULL)){
		return EBADF;
	}
	
	// lock the entry
	lock_acquire(curthread->t_filetable->ft_entries[fd]->ft_lock);	

	/* if the file entry has only one fd pointing to it, close file */
	if (curthread->t_filetable->ft_entries[fd]->ft_count == 1){
		vfs_close(curthread->t_filetable->ft_entries[fd]->ft_vnode);

		lock_release(curthread->t_filetable->ft_entries[fd]->ft_lock);
		lock_destroy(curthread->t_filetable->ft_entries[fd]->ft_lock);
		kfree(curthread->t_filetable->ft_entries[fd]);

		curthread->t_filetable->ft_entries[fd] = NULL; // clear the entries

	} else if (curthread->t_filetable->ft_entries[fd]->ft_count > 1){
		curthread->t_filetable->ft_entries[fd]->ft_count--;
		lock_release(curthread->t_filetable->ft_entries[fd]->ft_lock);
		return ENOTEMPTY;
	}

	return 0;
}

/*** filetable functions ***/

/* 
 * filetable_init
 * pretty straightforward -- allocate the space, set up 
 * first 3 file descriptors for stdin, stdout and stderr,
 * and initialize all other entries to NULL.
 * 
 * Should set curthread->t_filetable to point to the
 * newly-initialized filetable.
 * 
 * Should return non-zero error code on failure.  Currently
 * does nothing but returns success so that loading a user
 * program will succeed even if you haven't written the
 * filetable initialization yet.
 */

int
filetable_init(void)
{
	/* File table already exists */
	if (curthread->t_filetable != NULL){
		return EBADF;
	}
	
	/* Allocate the memory */
	curthread->t_filetable = (struct filetable *)kmalloc(sizeof(struct filetable));
	if (curthread->t_filetable == NULL){
		return ENOMEM;
	}

	/* initialize the file descriptors */
	int fd;
	int result;
	char path[5];
	
	/* initiliaze the rest of file table entries to NULL */
	for (fd = 0; fd < __OPEN_MAX; fd++){
		curthread->t_filetable->ft_entries[fd] = NULL;
	}

	strcpy(path, "con:");
	result = file_open(path, O_RDONLY, 0, &fd);
	if (result){
		return result;
	}
	strcpy(path, "con:");
	result = file_open(path, O_WRONLY, 0, &fd);
	if (result){
		return result;
	}
	strcpy(path, "con:");
	result = file_open(path, O_WRONLY, 0, &fd);
	if (result){
		return result;
	}

	return 0;
}



/*
 * filetable_destroy
 * closes the files in the file table, frees the table.
 * This should be called as part of cleaning up a process (after kill
 * or exit).
 */
void
filetable_destroy(struct filetable *ft)
{
        int fd;
	for (fd = 0; fd < __OPEN_MAX; fd++){
		struct filetable_entry *fte = ft->ft_entries[fd];
		if (fte == NULL){
			file_close(fd);
		}
	}

	kfree(ft);
}	


/* 
 * You should add additional filetable utility functions here as needed
 * to support the system calls.  For example, given a file descriptor
 * you will want some sort of lookup function that will check if the fd is 
 * valid and return the associated vnode (and possibly other information like
 * the current file position) associated with that open file.
 */

/*
 * filetable_searchfile
 *
 */
int filetable_searchfile(struct filetable_entry **fe, int fd)
{
    if (fd >= __OPEN_MAX || fd < 0){
        return EBADF;
    }
    if (!(*fe = curthread->t_filetable->ft_entries[fd])){
        kprintf("meh\n");
        return EBADF;
    }

    return 0;
}


/* END A3 SETUP */
