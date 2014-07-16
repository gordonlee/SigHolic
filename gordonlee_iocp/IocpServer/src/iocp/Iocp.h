// Copyright 2014 GordonLee#pragma once
#pragma once 

#include <process.h>

#include "common_header/base_header.h"


class CIocp
{
public:
	CIocp();
	virtual ~CIocp();

	int StartIocpThread(DWORD _numberOfThread);
	int BindSocket(HANDLE _socket, DWORD _completionKey);

private:
    HANDLE m_iocpHandle;
};

