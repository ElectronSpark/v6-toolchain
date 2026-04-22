/* xv6 override of musl's __unmapself for x86_64.
 *
 * Upstream musl hardcodes Linux syscall numbers (11=munmap, 60=exit).
 * xv6 uses custom syscall numbers: SYS_munmap=51, SYS_exit=3.
 *
 * This function is called during pthread_exit / thread cleanup.
 * It unmaps the thread's own stack, then exits the thread.
 * Neither syscall should return — munmap only unmaps the mapping,
 * and exit terminates the calling thread.
 *
 * Arguments (already set by caller in __pthread_exit):
 *   rdi = base address of mapping to unmap
 *   rsi = length of mapping
 */
.text
.global __unmapself
.type   __unmapself,@function
__unmapself:
	movl $51,%eax   /* xv6 SYS_munmap */
	syscall         /* munmap(rdi, rsi) */
	xor %rdi,%rdi   /* exit status = 0 */
	movl $3,%eax    /* xv6 SYS_exit */
	syscall         /* exit(0) — never returns */
