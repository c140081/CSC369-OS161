/*
 * Process-related syscalls.
 * New for ASST1.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <current.h>
#include <pid.h>
#include <machine/trapframe.h>
#include <syscall.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <copyinout.h>
#include <kern/signal.h>
#include <limits.h>



/*
 * sys_fork
 *
 * create a new process, which begins executing in md_forkentry().
 */
int
sys_fork(struct trapframe *tf, pid_t *retval)
{
	struct trapframe *ntf; /* new trapframe, copy of tf */
	int result;
    
	/*
	 * Copy the trapframe to the heap, because we might return to
	 * userlevel and make another syscall (changing the trapframe)
	 * before the child runs. The child will free the copy.
	 */
    
	ntf = kmalloc(sizeof(struct trapframe));
	if (ntf==NULL) {
		return ENOMEM;
	}
	*ntf = *tf; /* copy the trapframe */
    
	result = thread_fork(curthread->t_name, enter_forked_process,
                         ntf, 0, retval);
	if (result) {
		kfree(ntf);
		return result;
	}
    
	return 0;
}


/*
 * sys_getpid - return the pid of the current process.
 */
int sys_getpid(pid_t *retvalue) {
    //Get pid and set *retval
    *retvalue = curthread->t_pid;
    return 0;
}

/*
 * sys_waitpid - wait for input process to exit and return encoded exit status in
 * user_status. Retuurn the pid of that process or 0 if WNOHANG option is given.
 * Error code returned for relevant error.
 */
int sys_waitpid(pid_t pid, userptr_t user_status, int options, pid_t *retvalue) {
    int status = 0;
    
    //Case: invalid options
    if ((options != 0) && (options != WNOHANG)) {
        return EINVAL;
    }
    
    // status points to NULL
    if (! user_status || user_status == INVAL_PTR || user_status == KERN_PTR) {
        return EFAULT;
    }
    
    // pid doesn't exist
    if (pid == INVALID_PID) {
        return ESRCH;
    }

    
    
    int exitstatus = pid_join(pid, &status, options);
    copyout(&status, user_status, sizeof(int));
    // If exitstatus from pid_join is negative, return the error. Else set retvalue.
    if (exitstatus >= 0) {
        *retvalue = exitstatus;
        return 0;
    } else {
        //*retvalue = -exitstatus;
        return -exitstatus;
    }
    

}

/*
 * sys_kill - send input signal to input pid.
 * On success, return 0. On error, return error code.
 */
int sys_kill(pid_t pid, int signal) {
    
    // Attempt to set flag.
    int retvalue = pid_flag_activate(pid, signal);
    
    // If successful, return 0.
    if (retvalue >= 0) {
        return retvalue;
    // Else return the error code.
    } else {
        return -retvalue;
    }
}
