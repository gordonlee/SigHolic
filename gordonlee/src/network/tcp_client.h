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
class PulledBuffer;

struct _OverlappedIO;
typedef struct _OverlappedIO OverlappedIO;

class TcpClient : public ITcpClient {
 public:	
     // MEMO: Regular Functions
     TcpClient(void);
     virtual ~TcpClient( void );
     void OnAccept();

	 int ReceiveAsync( void );
	 void OnReceived(unsigned long transferred);

	 void TryProcessPacket(void);

 public:	
     // MEMO: Accessors and Mutators (getter/setter)
     SOCKET socket() { return socket_; }
     OverlappedIO* accept_overlapped() { return accept_overlapped_; }
     SessionStatus status() { return status_; }
     byte* accept_buffer() { return accept_buffer_; }
     DWORD* accept_bytes() { return &accept_bytes_; }

 private:
     SOCKET socket_;
	 
	 //
     SOCKADDR_IN local_addr_;
     SOCKADDR_IN remote_addr_;
     SessionStatus status_;
     byte accept_buffer_[1024];
     DWORD accept_bytes_;

     // for acceptex
     OverlappedIO* accept_overlapped_;

	 // recv
	 OverlappedIO* receive_overlapped_;
	 bool is_receiving_;
	 PulledBuffer* receive_buffer_;
	 int receive_bytes_;
};