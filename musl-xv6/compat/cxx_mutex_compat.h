/*
 * Minimal std::mutex / std::condition_variable stubs for xv6.
 *
 * libstdc++ was built without _GLIBCXX_HAS_GTHREADS, so <mutex> and
 * <condition_variable> define nothing for mutex/condvar.  ICU (and
 * potentially WebKit) need at least std::mutex.  musl *does* supply
 * pthreads, so we implement the classes directly on top of POSIX.
 *
 * Note: std::lock_guard and std::unique_lock are provided by libstdc++
 * unconditionally (they are generic templates), so we do NOT define
 * them here.
 */
#ifndef XV6_CXX_MUTEX_COMPAT_H
#define XV6_CXX_MUTEX_COMPAT_H

#include <pthread.h>

#ifndef _GLIBCXX_HAS_GTHREADS   /* only provide stubs when missing */

namespace std {

class mutex {
public:
    constexpr mutex() noexcept : _m(PTHREAD_MUTEX_INITIALIZER) {}
    ~mutex() { pthread_mutex_destroy(&_m); }
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;
    void lock()     { pthread_mutex_lock(&_m); }
    void unlock()   { pthread_mutex_unlock(&_m); }
    bool try_lock() { return pthread_mutex_trylock(&_m) == 0; }
    typedef pthread_mutex_t* native_handle_type;
    native_handle_type native_handle() { return &_m; }
private:
    pthread_mutex_t _m;
};

class condition_variable {
public:
    condition_variable() noexcept { pthread_cond_init(&_c, nullptr); }
    ~condition_variable() { pthread_cond_destroy(&_c); }
    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;
    void notify_one() noexcept { pthread_cond_signal(&_c); }
    void notify_all() noexcept { pthread_cond_broadcast(&_c); }
    template <class Lock>
    void wait(Lock& lk) {
        pthread_cond_wait(&_c, lk.mutex()->native_handle());
    }
private:
    pthread_cond_t _c;
};

}  // namespace std

#endif /* !_GLIBCXX_HAS_GTHREADS */

/*
 * std::scoped_lock (C++17) — libstdc++ without gthreads may not provide it.
 * WebKitGTK's bmalloc requires it.  Only the single-mutex specialization
 * is implemented, which is all that bmalloc uses.
 */
#if __cplusplus >= 201703L
namespace std {

template <class... MutexTypes>
class scoped_lock;

template <class Mutex>
class scoped_lock<Mutex> {
public:
    using mutex_type = Mutex;
    explicit scoped_lock(Mutex& m) : _m(m) { _m.lock(); }
    ~scoped_lock() { _m.unlock(); }
    scoped_lock(const scoped_lock&) = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;
private:
    Mutex& _m;
};

template <>
class scoped_lock<> {
public:
    explicit scoped_lock() = default;
    ~scoped_lock() = default;
    scoped_lock(const scoped_lock&) = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;
};

template <class... MutexTypes>
scoped_lock(MutexTypes&...) -> scoped_lock<MutexTypes...>;

} // namespace std
#endif /* C++17 */

#endif /* XV6_CXX_MUTEX_COMPAT_H */
