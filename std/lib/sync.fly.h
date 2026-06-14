namespace fly.sync

public struct Thread {
    int id
}

public struct Mutex {
    long ptr
}

public struct RWMutex {
    long state_ptr
    long waiters_ptr
}

public struct WaitGroup {
    long ptr
}

public struct Once {
    long ptr
}

public struct Semaphore {
    long ptr
}

public Thread spawnThread(const long fn, const long arg, const ulong stack)
public Mutex newMutex()
public mutexLock(Mutex m)
public mutexUnlock(Mutex m)
public bool mutexTryLock(Mutex m)
public RWMutex newRWMutex()
public rwmutexRlock(RWMutex rw)
public rwmutexRunlock(RWMutex rw)
public rwmutexWlock(RWMutex rw)
public rwmutexWunlock(RWMutex rw)
public WaitGroup newWaitGroup()
public wgAdd(WaitGroup wg, const int n)
public wgDone(WaitGroup wg)
public wgWait(WaitGroup wg)
public Once newOnce()
public bool onceAcquire(Once o)
public onceRelease(Once o)
public Semaphore newSemaphore(const int initial)
public semaphoreAcquire(Semaphore s)
public semaphoreRelease(Semaphore s)
public bool semaphoreTryAcquire(Semaphore s)
