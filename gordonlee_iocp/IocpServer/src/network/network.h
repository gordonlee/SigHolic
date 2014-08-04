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

unsigned int __stdcall WorkerThread(LPVOID lpParam);

class Network sealed {
 public:
	 Network(void);
	 ~Network(void);

	 void Initialize(void);
	 void Run(void);
     void RunWithAcceptEx(void);
	 void Cleanup();

 private:
	 bool StartNetwork();
	 void EndNetwork();

 private:
     TcpServer m_ListenSocket;
     CIocp m_Iocp;
};
