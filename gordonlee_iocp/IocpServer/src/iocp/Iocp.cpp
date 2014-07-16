// Copyright 2014 GordonLee

#include "Iocp.h"

extern unsigned int __stdcall WorkerThread(LPVOID arg);

CIocp::CIocp() {
}
CIocp::~CIocp() {
}

int CIocp::StartIocpThread(DWORD _numberOfThread) {
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

int CIocp::BindSocket(HANDLE _socket, DWORD _completionKey) {
	// FIXME: �Ѵ� �����ε�.. api �ڼ��� �о�� ���� �ٽ� �� ��.
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