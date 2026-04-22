/* xv6 override of musl's __unmapself for riscv64.
 *
 * Upstream musl hardcodes Linux syscall numbers (215=munmap, 93=exit).
 * xv6 uses custom syscall numbers: SYS_munmap=51, SYS_exit=3.
 *
 * On xv6, Linux's 93 maps to SYS_dumppcache (hence the "Dumping all
 * pcache stats" output) and 215 is undefined, so the thread never
 * actually unmaps or exits — it falls through to sepc=0x0.
 *
 * This function is called during pthread_exit / thread cleanup.
 * It unmaps the thread's own stack, then exits the thread.
 * Neither syscall should return — munmap only unmaps the mapping,
 * and exit terminates the calling thread.
 *
 * Arguments (already set by caller in __pthread_exit):
 *   a0 = base address of mapping to unmap
 *   a1 = length of mapping
 */
.global __unmapself
.type __unmapself, %function
__unmapself:
	li a7, 51   /* xv6 SYS_munmap */
	ecall       /* munmap(a0, a1) */
	li a0, 0    /* exit status = 0 */
	li a7, 3    /* xv6 SYS_exit */
	ecall       /* exit(0) — does not return */
