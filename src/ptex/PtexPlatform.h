#ifndef PtexPlatform_h
#define PtexPlatform_h
#define PtexPlatform_h
/*
PTEX SOFTWARE
Copyright 2014 Disney Enterprises, Inc.  All rights reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

  * The names "Disney", "Walt Disney Pictures", "Walt Disney Animation
    Studios" or the names of its contributors may NOT be used to
    endorse or promote products derived from this software without
    specific prior written permission from Walt Disney Pictures.

Disclaimer: THIS SOFTWARE IS PROVIDED BY WALT DISNEY PICTURES AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT AND TITLE ARE DISCLAIMED.
IN NO EVENT SHALL WALT DISNEY PICTURES, THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND BASED ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*/

/** @file PtexPlatform.h
    @brief Platform-specific classes, functions, and includes.
*/

#include <inttypes.h>

// platform-specific includes
#if defined(_WIN32) || defined(_WINDOWS) || defined(_MSC_VER)
#ifndef WINDOWS
#define WINDOWS
#endif
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_SECURE_NO_DEPRECATE 1
#define NOMINMAX 1

// windows - defined for both Win32 and Win64
#include <Windows.h>
#include <malloc.h>
#include <io.h>
#include <tchar.h>
#include <process.h>

#else

// linux/unix/posix
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <pthread.h>

#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#include <sys/types.h>
#endif
#endif

// general includes
#include <stdio.h>
#include <cmath>
#include <assert.h>

// missing functions on Windows
#ifdef WINDOWS
typedef __int64 FilePos;
#define fseeko _fseeki64
#define ftello _ftelli64

#else
typedef off_t FilePos;
#endif

#include "PtexVersion.h"

PTEX_NAMESPACE_BEGIN

/*
 * Mutex
 */

#ifdef WINDOWS

class Mutex {
public:
    Mutex()       { _mutex = CreateMutex(NULL, FALSE, NULL); }
    ~Mutex()      { CloseHandle(_mutex); }
    void lock()   { WaitForSingleObject(_mutex, INFINITE); }
    void unlock() { ReleaseMutex(_mutex); }
private:
    HANDLE _mutex;
};

class SpinLock {
public:
    SpinLock()    { InitializeCriticalSection(&_spinlock); }
    ~SpinLock()   { DeleteCriticalSection(&_spinlock); }
    void lock()   { EnterCriticalSection(&_spinlock); }
    void unlock() { LeaveCriticalSection(&_spinlock); }
private:
    CRITICAL_SECTION spinlock;
};

#else
// assume linux/unix/posix

class Mutex {
public:
    Mutex()      { pthread_mutex_init(&_mutex, 0); }
    ~Mutex()     { pthread_mutex_destroy(&_mutex); }
    void lock()   { pthread_mutex_lock(&_mutex); }
    bool trylock() { return 0 == pthread_mutex_trylock(&_mutex); }
    void unlock() { pthread_mutex_unlock(&_mutex); }
private:
    pthread_mutex_t _mutex;
};

#ifdef __APPLE__
class SpinLock {
public:
    SpinLock()   { _spinlock = 0; }
    ~SpinLock()  { }
    void lock()   { OSSpinLockLock(&_spinlock); }
    void unlock() { OSSpinLockUnlock(&_spinlock); }
private:
    OSSpinLock _spinlock;
};
#else
class SpinLock {
public:
    SpinLock()   { pthread_spin_init(&_spinlock, PTHREAD_PROCESS_PRIVATE); }
    ~SpinLock()  { pthread_spin_destroy(&_spinlock); }
    void lock()   { pthread_spin_lock(&_spinlock); }
    bool trylock() { return 0 == pthread_spin_trylock(&_spinlock); }
    void unlock() { pthread_spin_unlock(&_spinlock); }
private:
    pthread_spinlock_t _spinlock;
};
#endif // __APPLE__
#endif

/*
 * Atomics
 */

#if defined(WINDOWS)
// TODO Windows atomics

#elif defined(__APPLE__)
// TODO OSX atomics

#else
// assume linux/unix/posix

template <typename T>
inline T AtomicIncrement(volatile T* target)
{
    return __sync_add_and_fetch(target, 1);
}

template <typename T>
inline T AtomicAdd(volatile T* target, T value)
{
    return __sync_add_and_fetch(target, value);
}

template <typename T>
inline T AtomicDecrement(volatile T* target)
{
    return __sync_sub_and_fetch(target, 1);
}

template <typename T>
inline bool AtomicCompareAndSwap(T volatile* target, T oldvalue, T newvalue)
{
    return __sync_bool_compare_and_swap(target, oldvalue, newvalue);
}

template <typename T>
inline void AtomicStore(T volatile* target, T value)
{
    __sync_synchronize();
    *target = value;
}

inline void MemoryFence()
{
    __sync_synchronize();
}

#endif

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

#define PAD(var) char var##_pad[CACHE_LINE_SIZE - sizeof(var)]
#define PAD_INIT(var) memset(&var##_pad[0], 0, sizeof(var##_pad))

PTEX_NAMESPACE_END

#endif // PtexPlatform_h
