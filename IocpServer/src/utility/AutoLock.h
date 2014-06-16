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

//MEMO: m_lock�� ������ü�̱� ������, �ٸ� ������ ���� �ΰ��̻� ���� �� ���� ������ �ȴ�. 
//�� ����� �� ��.
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

//TODO: ���Ǹ� �̷��� �ؾ��ϴ°� ��.. �ٲ� �� ������?
Lock AutoLock::m_lock;
