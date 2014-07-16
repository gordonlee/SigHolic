#include "TcpServer.h"
#include "../common_header/base_header.h"

#include <MSWSock.h>
#include <stdio.h>
TcpServer::TcpServer(void) {

}

TcpServer::~TcpServer(void) {

}

SOCKET TcpServer::Bind(int _listenPort) {
	m_Socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_Socket == INVALID_SOCKET) {
		//TODO: log here.
		return -1;
	}

	m_SocketAddr.sin_family = AF_INET;
	m_SocketAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	m_SocketAddr.sin_port = ::htons(_listenPort);

	::bind(m_Socket,
		reinterpret_cast<SOCKADDR *>(&m_SocketAddr),
		sizeof(m_SocketAddr));

	int result = ::listen(m_Socket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}
	return m_Socket;
}

BOOL TcpServer::AcceptEx(SOCKET _acceptSocket, 
	PVOID _lpOutputBuffer, 
	DWORD _recvDataLength, 
	DWORD _localAddrLength, 
	DWORD _remoteAddrLength, 
    LPOVERLAPPED _pOverlapped) {

    _recvDataLength = 0;
    _localAddrLength = _remoteAddrLength = sizeof(sockaddr_in)+16;

    char tempBuffer[1024] = { 0, };
	DWORD outReceivedBytes = 0;

	BOOL acceptResult = ::AcceptEx(
		m_Socket,
		_acceptSocket,
        tempBuffer,
		_recvDataLength,
		_localAddrLength,
		_remoteAddrLength,
		&outReceivedBytes,
		_pOverlapped);
    printf("reg clientSocket %d\n", _acceptSocket);
    return acceptResult;
}

SOCKET TcpServer::Accept(SOCKADDR_IN* _remoteAddr, int* _remoteLength) {
	SOCKET acceptedSocket = ::accept(
		m_Socket,
		reinterpret_cast<SOCKADDR*>(_remoteAddr),
		_remoteLength);

	return acceptedSocket;
}

void TcpServer::Close(bool _isForce) {
	::closesocket(m_Socket);
}

SOCKET TcpServer::GetSocket() {
	return m_Socket;
}