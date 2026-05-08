/*
 * __clone wrapper for xv6 x86_64
 *
 * musl's pthread_create calls __clone(func, stack, flags, arg, ptid, tls, ctid)
 * C calling convention: rdi=func, rsi=stack, rdx=flags, rcx=arg, r8=ptid, r9=tls
 *                       and ctid is on the stack at 8(%rsp)
 *
 * xv6 now accepts the native Linux x86_64 clone syscall ABI:
 *   clone(flags, child_stack, parent_tid, child_tid, tls)
 *
 * Linux x86-64 syscall convention:
 *   rcx and r11 are clobbered by SYSCALL (transaction registers).
 *   After the syscall instruction, rcx/r11 no longer hold their
 *   pre-syscall values.  This is fine because this function consumes
 *   rcx (=arg) before issuing syscall, and rcx/r11 are caller-saved
 *   in the C ABI.
 */

.global __clone
.hidden __clone
.type __clone, @function
__clone:
    /* Arguments from musl C ABI:
     *   rdi = func,  rsi = stack,  rdx = flags,
     *   rcx = arg,   r8  = ptid,   r9  = tls
     *   8(%rsp) = ctid
     */

    /* Save func and arg on the child's stack (it grows down) */
    subq $16, %rsi          /* reserve 16 bytes at top of child stack */
    movq %rdi, 0(%rsi)      /* child_stack[0] = func */
    movq %rcx, 8(%rsi)      /* child_stack[1] = arg */

    /* ctid is the 7th musl wrapper arg, on the stack */
    movq 8(%rsp), %rax      /* rax = ctid */
    movq %rax, %r10          /* Linux clone arg4 = child_tid */
    movq %rdx, %rdi          /* Linux clone arg1 = flags */
    movq %r8,  %rdx          /* Linux clone arg3 = parent_tid */
    movq %r9,  %r8           /* Linux clone arg5 = tls */
    movq $56, %rax           /* SYS_clone */
    syscall

    /* Check return value */
    testq %rax, %rax
    jz    .Lchild
    /* Parent: return child pid (or error) */
    ret

.Lchild:
    /* Child returns here with rsp = child stack set by kernel.
     * The kernel set rsp to clone_args.stack, which has:
     *   0(%rsp) = func
     *   8(%rsp) = arg
     */
    xorq %rbp, %rbp          /* mark end of frames */
    popq %rax                 /* rax = func */
    popq %rdi                 /* rdi = arg */
    /* Ensure 16-byte stack alignment before call (x86_64 ABI).
     * musl only guarantees 8-byte alignment for the child stack.
     * callq will push 8 bytes, so RSP must be 16-aligned now. */
    andq $-16, %rsp
    callq *%rax               /* func(arg) */

    /* func returned — call exit_group */
    movq %rax, %rdi
    movq $231, %rax           /* SYS_exit_group */
    syscall
    hlt
