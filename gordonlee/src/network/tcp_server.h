#pragma once 
#include "core_header\base_header.h"
#include "network\iocp_structure.h"

class TcpClient;

class TcpServer {
 public:
     TcpServer(void);
     virtual ~TcpServer(void);
     
     bool Start(void);

     bool AcceptEx(TcpClient* _client);
     SOCKET Accept(SOCKADDR_IN* _remote_addr, int* _remote_length);

 public:
     void set_listen_port(int _port) { listen_port_ = _port; }
     SOCKET socket() { return socket_; }

 private:
     bool Initialize();
     bool Bind();
     bool Listen();

 private:
     SOCKET socket_;
     SOCKADDR_IN socket_addr_;
     int listen_port_;
     OverlappedIO accept_io_;
     bool is_started;
};