// Copyright 2014 GordonLee
#pragma once 

// FIXME: ��� ���� �� ������?
#include "../common_header/base_header.h"

class TcpServer {
 public:
	TcpServer(void);
	virtual ~TcpServer(void);

	SOCKET Bind(int _listenPort);

    BOOL AcceptEx(SOCKET _acceptSocket,
        PVOID _lpOutputBuffer,
        DWORD _recvDataLength,
        DWORD _localAddrLength,
        DWORD _remoteAddrLength,
        LPOVERLAPPED _pOverlapped);

	SOCKET Accept(SOCKADDR_IN* _remoteAddr, int* _remoteLength);
	
	void Close(bool _isForce);
	
 public:
	SOCKET GetSocket();

 private:
	SOCKET m_Socket;
	SOCKADDR_IN m_SocketAddr;
};
