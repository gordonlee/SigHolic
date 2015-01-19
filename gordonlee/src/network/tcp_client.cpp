#include "tcp_client.h"

#include <Mswsock.h>
#include <Ws2tcpip.h>

namespace
{

}

TcpClient::TcpClient(void) {

    ::memset(accept_buffer_, 0, 1024);

    accept_overlapped_ = new OverlappedIO();
    accept_overlapped_->set_session(std::shared_ptr<TcpClient>(this));

    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

TcpClient::~TcpClient(void) {
    if (accept_overlapped_ != NULL) {
        delete accept_overlapped_;
        accept_overlapped_ = NULL;
    }
}

// TODO: The function should support not only AcceptEx version, but also accept version too.
///		Now, it supports only AcceptEx version.
void TcpClient::OnAccept() {

    DWORD local_addr_length = sizeof(sockaddr_in)+16, remote_addr_length = sizeof(sockaddr_in)+16;

    int out_local_addr_length = 0, out_remote_addr_length = 0;

    SOCKADDR_IN* local_addr = NULL;
    SOCKADDR_IN* remote_addr = NULL;

    ::GetAcceptExSockaddrs(
        accept_buffer_, 
        accept_bytes_, 
        local_addr_length, 
        remote_addr_length,
        (SOCKADDR**)&local_addr, 
        &out_local_addr_length,
        (SOCKADDR**)&remote_addr, 
        &out_remote_addr_length);

	::memcpy(&local_addr_, local_addr, out_local_addr_length);
	::memcpy(&remote_addr_, remote_addr, out_remote_addr_length);

	char test_buffer[16] = { 0, };
	inet_ntop(AF_INET, &(remote_addr_.sin_addr), test_buffer, 16);

	printf("[TCP Server] Client Accepted: IP=%s, Port=%d\n",
		test_buffer, ntohs(remote_addr_.sin_port));
	
	status_ = CONNECTED;
}

