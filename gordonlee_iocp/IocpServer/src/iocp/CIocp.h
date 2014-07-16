// Copyright 2014 GordonLee#pragma once
#include <process.h>

extern unsigned int __stdcall WorkerThread(LPVOID arg);

class CIocp
{
public:
    CIocp(){}
    ~CIocp(){}

    int StartIocpThread(DWORD _numberOfThread) {
        HANDLE iocpHandle = ::CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            NULL,
            0,
            0);
        if (iocpHandle == NULL)
        {
            //TODO: fail log here
            return -1;
        }
        m_iocpHandle = iocpHandle;

        HANDLE hThread;
        unsigned int threadId = 0;
        for (DWORD i = 0; i < _numberOfThread; ++i)
        {
            hThread = (HANDLE)_beginthreadex(
                0,
                0,
                WorkerThread,
                iocpHandle,
                0,
                &threadId);
            if (hThread == NULL)
            {
                return -2;
            }
            ::CloseHandle(hThread);
        }
        return 0;
    }

    int BindSocket(HANDLE _socket, DWORD _completionKey) {
        // FIXME: 둘다 소켓인데.. api 자세히 읽어보고 개념 다시 볼 것.
        HANDLE hResult = ::CreateIoCompletionPort(
            _socket,
            m_iocpHandle,
            _completionKey,
            0);
        if (hResult == NULL) {
            return -1;
        }
        return 0;
    }

private:
    HANDLE m_iocpHandle;
};

