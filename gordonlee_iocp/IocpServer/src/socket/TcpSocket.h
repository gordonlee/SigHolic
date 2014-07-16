// Copyright 2014 GordonLee
#pragma once


#include "common_header/base_header.h"

class TcpSocket {
 public:
     TcpSocket(void) {
     }

     virtual ~TcpSocket(void) {
     }

     virtual int Initialize(void) {
         return 0;
     }

     virtual int Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen) {
         m_Socket = _socket;
         ::memcpy(&(m_SocketAddr), &_addr, _addrLen);

         int addrlen = sizeof(m_SocketAddr);
         getpeername(m_Socket, (SOCKADDR *)&m_SocketAddr, &addrlen);
         m_IsSetupAddr = true;
         return 0;
     }

     // Create Listen socket
     virtual int Bind(int _listenPort) {
         m_Socket = ::socket(AF_INET, SOCK_STREAM, 0);
         if (m_Socket == INVALID_SOCKET) {
             //TODO: log here.
             return -1;
         }

         m_SocketAddr.sin_family = AF_INET;
         m_SocketAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
         m_SocketAddr.sin_port = ::htons(_listenPort);
         m_IsSetupAddr = true;

         ::bind(m_Socket,
             reinterpret_cast<SOCKADDR *>(&m_SocketAddr),
             sizeof(m_SocketAddr));

         int result = ::listen(m_Socket, SOMAXCONN);
         if (result == SOCKET_ERROR) {
             return SOCKET_ERROR;
         }
         return m_Socket;
     }

     virtual SOCKET Accept(SOCKADDR_IN* _remoteAddr, int* _remoteLength) {
         SOCKET acceptedSocket = ::accept(
             m_Socket,
             reinterpret_cast<SOCKADDR*>(_remoteAddr),
             _remoteLength);

         return acceptedSocket;
     }

     virtual int SendAsync(void) {
         return 0;
     }

     virtual int Send(Buffer* _buffer, int _sendBytes) {
         if (_buffer && _sendBytes > 0) {
             return ::send(m_Socket, _buffer->GetPtr(), _sendBytes, 0);
         }
         return -1;
     }

     virtual int RecvAsync(Buffer* _buffer, const LPOVERLAPPED _overlapped) {
         static DWORD recvBytes;
         static DWORD flags = 0;
         // TODO: see the result.
         static WSABUF wsaBuf;
         wsaBuf.buf = _buffer->GetPtr();
         wsaBuf.len = BUFFER_SIZE;

         int result = ::WSARecv(
             m_Socket,
             &wsaBuf,
             1,
             &recvBytes,
             &flags,
             _overlapped,
             NULL);

         if (result == SOCKET_ERROR){
             if (WSAGetLastError() != ERROR_IO_PENDING) {
                 return -1;
             }
         }
         return 0;
     }

     virtual void Close(bool isForce) {
         ::closesocket(m_Socket);
     }

     SOCKET GetSocket() {
         return m_Socket;
     }

 private:
     // SocketTraits m_Socket;
     SOCKET m_Socket;
     SOCKADDR_IN m_SocketAddr;
     bool m_IsSetupAddr;
};
