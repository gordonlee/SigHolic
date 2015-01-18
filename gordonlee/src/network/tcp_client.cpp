#include "tcp_client.h"

#include <Mswsock.h>

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

void TcpClient::OnAccept() {

    DWORD recv_data_length = 0, local_addr_length = sizeof(sockaddr_in)+16, remote_addr_length = sizeof(sockaddr_in)+16;

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

    /*
    int addr_len = sizeof(socket_addr_);
    int result = ::getpeername(socket_, (SOCKADDR*)&socket_addr_, &addr_len);
    if (result != 0) {
        // fail
        int was_error = ::WSAGetLastError();
        printf("%d\n", was_error);
    }
    else {
        printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(socket_addr_.sin_addr), ntohs(socket_addr_.sin_port));

        status_ = CONNECTED;
    }
    */
    status_ = CONNECTED;
}

