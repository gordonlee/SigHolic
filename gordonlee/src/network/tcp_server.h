#pragma once 
#include <MSWSock.h>
#include "core_header\base_header.h"
#include "network\iocp_structure.h"
#include "network\tcp_client.h"

class TcpServer {
 public:
     TcpServer(void) : socket_(0), listen_port_(9000), is_started(false) {
         accept_io_.Reset();
     }
     virtual ~TcpServer(void) {

     }
     
     bool Start(void) {
         // listening socket
         if ( Initialize() == false) {
             return false;
         }
         if (Bind() == false) {
             return false;
         }
         if (Listen() == false) {
             return false;
         }
         return true;
     }

     bool AcceptEx(TcpClient* _client) {
         char temp_buffer[1024] = { 0, };
         DWORD recv_data_length=0, local_addr_length=0, remote_addr_length=0;
         DWORD out_recv_bytes = 0;
         
         BOOL async_accept_result = ::AcceptEx(
             socket_,
             _client->socket(),
             temp_buffer,
             0,
             0,
             0,
             &out_recv_bytes,
             _client->accept_overlapped()
             );

         return async_accept_result;
     }

     SOCKET Accept(SOCKADDR_IN* _remote_addr, int* _remote_length) {
         SOCKET acceptedSocket = ::accept(
             socket_,
             reinterpret_cast<SOCKADDR*>(_remote_addr),
             _remote_length);
         if (acceptedSocket == INVALID_SOCKET) {
             printf("accept failed with error: %d\n", WSAGetLastError());
         }

         return acceptedSocket;
     }


 public:
     void set_listen_port(int _port) {
         listen_port_ = _port;
     }
     SOCKET socket() { return socket_; }

 private:

     bool Initialize() {
         socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
         if (socket_ == INVALID_SOCKET) {
             //TODO: log here.
             return false;
         }
         return true;
     }

     bool Bind() {
         socket_addr_.sin_family = AF_INET;
         socket_addr_.sin_addr.s_addr = ::htonl(INADDR_ANY);
         socket_addr_.sin_port = ::htons(listen_port_);

         int bindResult = ::bind(socket_,
             reinterpret_cast<SOCKADDR *>(&socket_addr_),
             sizeof(socket_addr_));
         if (bindResult == SOCKET_ERROR) {
             printf("bind failed with error: %d\n", WSAGetLastError());
             return false;
         }
         return true;
     }

     bool Listen() {
         int result = ::listen(socket_, SOMAXCONN);
         if (result == SOCKET_ERROR) {
             printf("listen failed with error: %d\n", WSAGetLastError());
             return false;
         }
         return true;
     }


 private:
     SOCKET socket_;
     SOCKADDR_IN socket_addr_;
     int listen_port_;
     OverlappedIO accept_io_;
     bool is_started;
};