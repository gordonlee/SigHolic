#include "tcp_client.h"

TcpClient::TcpClient(void) {
    accept_overlapped_ = new OverlappedIO();
    accept_overlapped_->set_session(std::shared_ptr<TcpClient>(this));
}

TcpClient::~TcpClient(void) {
    if (accept_overlapped_ != NULL) {
        delete accept_overlapped_;
        accept_overlapped_ = NULL;
    }
}

