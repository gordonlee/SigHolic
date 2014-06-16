// Copyright 2014 GordonLee#pragma once
#include <winsock2.h>


extern DWORD WINAPI WorkerThread(LPVOID arg);

class CIocp
{
public:
    CIocp(){}
    ~CIocp(){}

    int StartIocpThread(DWORD numberOfThread) {

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
        DWORD threadId = 0;
        for (DWORD i = 0; i < numberOfThread; ++i)
        {
            hThread = ::CreateThread(
                NULL,
                0,
                // &cIocps::WorkerThread,
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

    int BindSocket(HANDLE socket) {

        HANDLE hResult = ::CreateIoCompletionPort(
            socket,
            m_iocpHandle,
            (DWORD)socket,
            0);
        if (hResult == NULL) {
            return -1;
        }
        return 0;
    }

private:
    HANDLE m_iocpHandle;
};

