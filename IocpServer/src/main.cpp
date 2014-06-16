// Copyright 2014 GordonLee

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#include "iocp\CIocp.h"
#include "utility\AutoLock.h"

const int BUFSIZE = 512;
typedef enum IO_TYPE { IO_READ, IO_ACCEPT };

typedef struct _PER_HANDLE_DATA {
    SOCKET Socket;
    SOCKADDR_STORAGE ClientAddr;
    OVERLAPPED Overlapped;
    IO_TYPE IoType;
    byte Buffer[BUFSIZE];
    WSABUF WsaBuf;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

namespace
{
    //FIXME: replace this function to log class
    void err_display(char *msg)
    {
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf, 0, NULL);
        printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
}


DWORD WINAPI WorkerThread(LPVOID lpParam)
{
    //TODO: write down worker thread.
    HANDLE handle = (HANDLE)lpParam;
    int retval;
    DWORD cbTransferred;
    SOCKET client_socket;
    LPPER_HANDLE_DATA data = NULL;

    while (TRUE)
    {
        retval = ::GetQueuedCompletionStatus(
            handle,
            &cbTransferred,
            (LPDWORD)&client_socket,
            (LPOVERLAPPED *)(&data),
            INFINITE);

        SOCKADDR_IN clientaddr;
        int addrlen = sizeof(clientaddr);
        getpeername(data->Socket, (SOCKADDR *)&clientaddr, &addrlen);

        if (retval == 0 || cbTransferred == 0) {
            if (retval == 0) {
                DWORD temp1, temp2;
                ::WSAGetOverlappedResult(data->Socket, &(data->Overlapped), &temp1, FALSE, &temp2);
                err_display("WSAGetOverlappedResult()");
            }

            {
                AutoLock lock;
                // mamager.remove(socket);
            }

            closesocket(data->Socket);
            printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
                inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            delete data;
            continue;
        }

        if (data) {
            data->Buffer[cbTransferred] = NULL;
            // protocol process. 
            if (data->IoType == IO_ACCEPT) {
                printf("초기 데이터 : %s\n", data->Buffer);
                // printf("사이즈 : %d\n", g_sock.size());
                // 데이터 에코처리
                {
                    AutoLock lock;
                    /* 
                    //TODO: find socket -> send to client //echo
                    list<SOCKETINFO*>::iterator it = g_sock.begin();
                    while (it != g_sock.end())
                    {
                        SOCKETINFO*a = *it;
                        send(a->sock, ptr->buf, cbTransferred, 0);
                        it++;
                    }
                    */
                }
            }
            else if (data->IoType == IO_READ) {
                printf("READ 데이터 : %s\n", data->Buffer);
                {
                    AutoLock lock;
                    /*
                    //TODO: find socket -> send to client // echo 
                    list<SOCKETINFO*>::iterator it = g_sock.begin();
                    while (it != g_sock.end())
                    {
                        SOCKETINFO*a = *it;
                        send(a->sock, ptr->buf, cbTransferred, 0);
                        it++;
                    }
                    */
                }

            }

            ZeroMemory(&(data->Overlapped), sizeof(data->Overlapped));
            data->WsaBuf.buf = (char*)(data->Buffer);
            data->WsaBuf.len = BUFSIZE;
            data->IoType= IO_READ;

            DWORD recvbytes;
            DWORD flags = 0;
            retval = ::WSARecv(
                data->Socket, 
                &(data->WsaBuf), 
                1, 
                &recvbytes, 
                &flags, 
                &(data->Overlapped), 
                NULL);
            if (retval == SOCKET_ERROR){
                if (WSAGetLastError() != WSA_IO_PENDING){
                    err_display("WSARecv()1");
                }
                continue;
            }
        }
    }
    
    return 0;
}

int main() 
{
    WSADATA wsa;

    if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        // log here. 
        return -1;
    }

    CIocp iocp = CIocp();

    // 1. create I/O completion port
    const int numberOfThread = 4;   //TODO: remove magic number later.
    iocp.StartIocpThread(numberOfThread);

    // 4. create listen socket.
    SOCKET ListenSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    InternetAddr.sin_port = ::htons(5150);
    ::bind(ListenSocket, (SOCKADDR *)&InternetAddr, sizeof(InternetAddr));

    int result = ::listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        return -3;
    }

    while (TRUE)
    {
        PER_HANDLE_DATA *PerHandleData = NULL;
        
        // 5. accept connection 
        SOCKADDR_IN saRemote;
        int remoteLen = sizeof(saRemote);
        SOCKET Accepted = ::accept(ListenSocket, (SOCKADDR *)&saRemote, &remoteLen);

        // 6. create socket handle data struct
        PerHandleData = (LPPER_HANDLE_DATA) ::GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
        
        printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(saRemote.sin_addr), ntohs(saRemote.sin_port));
        printf("Socket number %d connected\n", Accepted);

        PerHandleData->Socket = Accepted;
        ::memcpy(&PerHandleData->ClientAddr, &saRemote, remoteLen);

        // 7. bind client socket handle to completion port.
        iocp.BindSocket((HANDLE)Accepted);
        // ::CreateIoCompletionPort((HANDLE)Accepted, CompletionPort, (DWORD)PerHandleData, 0);

        // 8. start socket's i/o job
        PerHandleData->WsaBuf.buf = (char*)(PerHandleData->Buffer);
        PerHandleData->WsaBuf.len = BUFSIZE;
        DWORD recvbytes;
        DWORD flags = 0;
        int retval = WSARecv(
            Accepted, 
            &(PerHandleData->WsaBuf), 
            1, 
            &recvbytes,
            &flags, 
            &(PerHandleData->Overlapped), 
            NULL);
        if (retval == SOCKET_ERROR){
            if (WSAGetLastError() != ERROR_IO_PENDING) {
                err_display("WSARecv()0");
            }
            continue;
        }
    }
    return 0;
}
