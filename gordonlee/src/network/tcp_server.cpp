#include "network\tcp_server.h"

#include <MSWSock.h>

#include "network\tcp_client.h"

void err_display(char *msg) {
    LPVOID lpMsgBuf;
    int errorCode = WSAGetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);
    printf("[%s][Code: %d] %s ", msg, errorCode, (LPCTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
}


TcpServer::TcpServer() 
: socket_(0)
, listen_port_(9000)
, is_started(false) {
    accept_io_.Reset();
}

TcpServer::~TcpServer() {

}

bool TcpServer::Start() {
    // listening socket
    if (Initialize() == false) {
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

bool TcpServer::AcceptEx(TcpClient* _client) {
    char temp_buffer[1024] = { 0, };
    DWORD recv_data_length = 0, local_addr_length = sizeof(sockaddr_in)+16, remote_addr_length = sizeof(sockaddr_in)+16;
    DWORD out_recv_bytes = 0;

    BOOL async_accept_result = ::AcceptEx(
        socket_,
        _client->socket(),
        temp_buffer,
        recv_data_length,
        local_addr_length,
        remote_addr_length,
        &out_recv_bytes,
        _client->accept_overlapped()
        );

    if (!async_accept_result) {
        int error_code = ::WSAGetLastError();
        if (error_code != ERROR_IO_PENDING) {
            // TODO: log here.
            err_display("acceptEx");
        }
    }

    return async_accept_result;
}

SOCKET TcpServer::Accept(SOCKADDR_IN* _remote_addr, int* _remote_length) {
    SOCKET acceptedSocket = ::accept(
        socket_,
        reinterpret_cast<SOCKADDR*>(_remote_addr),
        _remote_length);
    if (acceptedSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
    }

    return acceptedSocket;
}

bool TcpServer::Initialize() {
    socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET) {
        //TODO: log here.
        return false;
    }
    return true;
}

bool TcpServer::Bind() {
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

bool TcpServer::Listen() {
    int result = ::listen(socket_, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}