// Copyright 2014 GordonLee
// MEMO: Completely pass through Cpplint
#pragma once

#include "core_header/base_header.h"

class ILock {
public:
    virtual void Enter(void) = 0;
    virtual void Leave(void) = 0;
};

class CriticalSectionLock sealed : public ILock{
public:
    CriticalSectionLock(void) {
        ::memset(&m_cs, 0, sizeof(CRITICAL_SECTION));
        ::InitializeCriticalSection(&m_cs);
    }

    virtual ~CriticalSectionLock(void) {
        ::DeleteCriticalSection(&m_cs);
    }

    virtual void Enter(void) {
        ::EnterCriticalSection(&m_cs);
    }

    virtual void Leave(void) {
        ::LeaveCriticalSection(&m_cs);
    }

private:
    CRITICAL_SECTION m_cs;
};


class AutoLock {
public:
    explicit AutoLock(ILock* lock) : m_lock(lock) {
        m_lock = lock;
        if (m_lock != NULL) {
            m_lock->Enter();
        }
    }

    ~AutoLock(void) {
        if (m_lock != NULL) {
            m_lock->Leave();
        }
    }

private:
    ILock* m_lock;
};

// MEMO: multiple lock objects according to tagName (scoped lock)
#define SCOPED_LOCK(tagName)    \
    static CriticalSectionLock lock_tagName;    \
    AutoLock autoLock_tagName(&lock_tagName);
