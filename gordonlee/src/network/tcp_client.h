#pragma once
#include "core_header\base_header.h"


#include "network\iocp_structure.h"
enum SessionStatus {
    NOT_CONNECTED = 0,
    CONNECTING,
    CONNECTED,
};

class ITcpClient {
 public:

};

struct _OverlappedIO;
typedef struct _OverlappedIO OverlappedIO;

class TcpClient : public ITcpClient {
 public:	
     // MEMO: Regular Functions
     TcpClient(void);
     virtual ~TcpClient(void);

 public:	
     // MEMO: Accessors and Mutators (getter/setter)
     SOCKET socket() { return socket_; }
     OverlappedIO* accept_overlapped() { return accept_overlapped_; }
     SessionStatus status() { return status_; }

 private:
     SOCKET socket_;
     SOCKADDR_IN socket_addr_;
     SessionStatus status_;

     // for acceptex
     OverlappedIO* accept_overlapped_;
};