#include "tcp_client.h"

TcpClient::TcpClient(void) {
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
}

