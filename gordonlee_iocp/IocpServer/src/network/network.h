// Copyright 2014 Gordonlee
#pragma once

// FIXME: remove stdio.h
#include <stdio.h>

// FIXME: move this headers to cpp file.
#include "common_header/base_header.h"
#include "iocp/Iocp.h"
#include "utility/AutoLock.h"
#include "utility/Buffer.h"
#include "socket/TcpClient.h"
#include "socket/TcpServer.h"

typedef struct _PER_HANDLE_DATA {
    OVERLAPPED Overlapped;
    TcpClient Socket;
    // Buffer Buffer;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


unsigned int __stdcall WorkerThread(LPVOID lpParam);

class Network sealed {
 public:
	 Network(void);
	 ~Network(void);

	 void Initialize(void);
	 void Run(void);
     void RunWithAcceptEx(void);
	 void Cleanup();
     // FIXME: this is test code
     // TcpServer m_ListenSocket;

 private:
	 LPPER_HANDLE_DATA MakeClientSession(SOCKET _clientSocket, const SOCKADDR_IN& _addr, const int _addrLen);
	 bool StartNetwork();
	 void EndNetwork();



 private:
     // CIocp m_Iocp;
     TcpServer m_ListenSocket;
     CIocp m_Iocp;
};
