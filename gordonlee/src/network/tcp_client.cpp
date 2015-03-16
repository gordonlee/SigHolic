#include "tcp_client.h"

#include <Mswsock.h>
#include <Ws2tcpip.h>

#include "utility\pulled_buffer.h"

namespace
{

}

TcpClient::TcpClient(void)
: is_receiving_(false)
, receive_bytes_(0) {

    ::memset(accept_buffer_, 0, 1024);

    accept_overlapped_ = new OverlappedIO();
    accept_overlapped_->set_session(std::shared_ptr<TcpClient>(this));

	receive_overlapped_ = new OverlappedIO();
	receive_overlapped_->set_session(std::shared_ptr<TcpClient>(this));

	receive_buffer_ = new PulledBuffer();

    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

TcpClient::~TcpClient(void) {
    if (accept_overlapped_ != NULL) {
        delete accept_overlapped_;
        accept_overlapped_ = NULL;
    }

	if (receive_buffer_)
	{
		delete receive_buffer_;
		receive_buffer_ = NULL;
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

int TcpClient::ReceiveAsync(void) {
	if ( is_receiving_ ) {
		printf("RecvAsync.m_RecvIoState is not IO_CONNECTED. Check threading synchronization.\n");
	}

	is_receiving_ = true;

	DWORD recvBytes;
	DWORD flags = 0;

	WSABUF wsaBuf;
	wsaBuf.buf = receive_buffer_->GetEmptyPtr();
	wsaBuf.len = receive_buffer_->GetEmptyLength();

	receive_overlapped_->Reset();

	int result = ::WSARecv(
		socket_,
		&wsaBuf,
		1,
		&recvBytes,
		&flags,
		receive_overlapped_,
		NULL);

	if (result == SOCKET_ERROR){
		int error_code = WSAGetLastError();
		if (error_code != ERROR_IO_PENDING) {
			return error_code;
		}
	}
	return 0;
}

void TcpClient::OnReceived(unsigned long transferred) {
	
	receive_bytes_ += transferred;
	receive_buffer_->ForceAddLength(transferred);

	is_receiving_ = false;

	TryProcessPacket();
}

void TcpClient::TryProcessPacket(void) {
	throw;
}
