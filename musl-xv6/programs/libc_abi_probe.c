/*
 * libc_abi_probe.c - compile/link probe for xv6 musl ABI shims.
 *
 * This is built by the toolchain verification step.  It catches regressions
 * where musl's generated bits/syscall.h turns __NR_* aliases into
 * self-referential SYS_* macros, and it pulls in pthread teardown paths so
 * the toolchain links against the libc-provided __unmapself implementation.
 */
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef SYS_memfd_create
#error "SYS_memfd_create is missing"
#endif
#ifndef __NR_memfd_create
#error "__NR_memfd_create is missing"
#endif
#ifndef SYS_mlock2
#error "SYS_mlock2 is missing"
#endif
#ifndef __NR_mlock2
#error "__NR_mlock2 is missing"
#endif
#ifndef SYS_mlockall
#error "SYS_mlockall is missing"
#endif
#ifndef __NR_mlockall
#error "__NR_mlockall is missing"
#endif
#ifndef SYS_munlock
#error "SYS_munlock is missing"
#endif
#ifndef __NR_munlock
#error "__NR_munlock is missing"
#endif
#ifndef SYS_munlockall
#error "SYS_munlockall is missing"
#endif
#ifndef __NR_munlockall
#error "__NR_munlockall is missing"
#endif

static volatile uintptr_t sink;

static void *thread_main(void *arg)
{
    sink ^= (uintptr_t)arg;
    return (void *)0;
}

int main(void)
{
    pthread_t thread;
    long nums[] = {
        SYS_memfd_create, __NR_memfd_create,
        SYS_mlock2, __NR_mlock2,
        SYS_mlockall, __NR_mlockall,
        SYS_munlock, __NR_munlock,
        SYS_munlockall, __NR_munlockall,
    };

    for (unsigned i = 0; i < sizeof(nums) / sizeof(nums[0]); i++)
        sink += (uintptr_t)nums[i];

    if (pthread_create(&thread, 0, thread_main, (void *)0x1234) != 0)
        return 1;
    if (pthread_join(thread, 0) != 0)
        return 2;

    return sink == 0 ? 3 : 0;
}
