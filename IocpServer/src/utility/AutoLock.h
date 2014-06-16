// Copyright 2014 GordonLee
#pragma once

#include <windows.h>

class Lock sealed {
public:
    Lock(void) {
        ::memset(&m_cs, 0, sizeof(CRITICAL_SECTION));
        ::InitializeCriticalSection(&m_cs);
    }
    
    ~Lock(void) {
        ::DeleteCriticalSection(&m_cs);
    }

    void Enter(void) {
        ::EnterCriticalSection(&m_cs);
    }

    void Leave(void) {
        ::LeaveCriticalSection(&m_cs);
    }

private:
    CRITICAL_SECTION m_cs;
};

//MEMO: m_lock이 전역객체이기 때문에, 다른 종류의 락이 두개이상 생길 수 없는 구조가 된다. 
//더 고민해 볼 것.
class AutoLock {
public:
    AutoLock(void) {
        m_lock.Enter();
    }

    ~AutoLock(void) {
        m_lock.Leave();
    }

private:
    static Lock m_lock;
};

//TODO: 정의를 이렇게 해야하는게 좀.. 바꿀 수 없을까?
Lock AutoLock::m_lock;
