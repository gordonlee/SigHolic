
#include "network.h"

#include "socket/TcpSessionManager.h"
#include "iocp/iocp_structure.h"


namespace {
	// FIXME: replace this function to log class
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


	static CriticalSectionLock g_recvIoPoolLock;
    static OverlappedIoPool g_recvIoPool(&g_recvIoPoolLock);

	static CriticalSectionLock g_sendIoPoolLock;
	static OverlappedIoPool g_sendIoPool(&g_sendIoPoolLock);

	OverlappedIO* MakeClientSessionEx(SOCKET _clientSocket, const SOCKADDR_IN& _addr, const int _addrLen) {
		std::shared_ptr<TcpClient> client (new TcpClient());

		OverlappedIO* overlapped = g_recvIoPool.Dequeue();
		client->BindRecvOverlapped(overlapped);
		overlapped->SetClientObject(client);

		OverlappedIO* sendOverlapped = g_sendIoPool.Dequeue();
		if (sendOverlapped == NULL) {
			printf("MakeClientSessionEx> CriticalError!\n");
		}
		client->BindSendOverlapped(sendOverlapped);
		sendOverlapped->SetClientObject(client);

		client->Initialize(_clientSocket, _addr, _addrLen);

		TcpSessionManager.AddTcpClient(client);

		return overlapped;
	}
}

// data가 sendOverlapped 인지, recvOverlapped 인지 모른다.
void CloseSession(OverlappedIO* data) {
    // printf("Close Client [Client: %X, Overlapped: %X]\n", data->GetClientObject(), data);

    if (std::shared_ptr<TcpClient> client = data->GetClientObject()) {
        TcpSessionManager.RemoveTcpClient(client);

		if (OverlappedIO* recvOverlapped = client->GetRecvOverlapped()) {
			recvOverlapped->Reset();
			recvOverlapped->SetClientObject(NULL);
			g_recvIoPool.Enqueue(recvOverlapped);
		}

		if (OverlappedIO* sendOverlapped = client->GetSendOverlapped()) {
			sendOverlapped->Reset();
			sendOverlapped->SetClientObject(NULL);
			g_sendIoPool.Enqueue(sendOverlapped);
		}

        client->Close(false);
    }
    
	// ::CancelIoEx(m_Iocp.GetHandle(), data);
}

unsigned int __stdcall WorkerThread(LPVOID lpParam) {

	// TODO: write down worker thread.
	HANDLE handle = (HANDLE)lpParam;
	int retval;
	DWORD cbTransferred;
	SOCKET client_socket = 0;
	OverlappedIO* data = NULL;

	while (TRUE) {
		retval = ::GetQueuedCompletionStatus(
			handle,
			&cbTransferred,
			(PULONG_PTR)&client_socket,
			(LPOVERLAPPED *)(&data),
			INFINITE);

		// DWORD lastError = GetLastError();

        //printf("-- [retval: %d, transfer: %d, socket: %d, Client: %X, Overlapped: %X]\n"
        //    , retval, cbTransferred, client_socket, data->GetClientObject(), data);

        /*
        if (retval > 0 &&
            (PULONG_PTR)client_socket == (PULONG_PTR)(&m_ListenSocket) &&
            data != NULL) {
            err_display("Acceped is succeeded.");
            printf("accepted socket %d\n", data->Socket.GetSocket());
            // OnAccepted
            // data->OnAcceped();

            ::ZeroMemory(&(data->Overlapped), sizeof(data->Overlapped));
            
            m_Iocp.BindSocket((HANDLE)(data->Socket.GetSocket()), static_cast<DWORD>(data->Socket.GetSocket()));

            if (data->Socket.RecvAsync(&(data->Buffer), &(data->Overlapped)) != 0) {
                err_display("WSARecv() Error at WorkerThread.");
            }
        }

        else
        */

        if (retval == 0 || cbTransferred == 0) {
			if (retval == 0) 
            {
				DWORD temp1, temp2;
				if (data != NULL)
				{
					::WSAGetOverlappedResult(
						data->GetClientObject()->GetSocket(),
						(LPWSAOVERLAPPED)&(data),
						&temp1,
						FALSE,
						&temp2);
				}

				err_display("WSAGetOverlappedResult()");
			}

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
            getpeername(data->GetClientObject()->GetSocket(), (SOCKADDR *)&clientaddr, &addrlen);

			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

            CloseSession(data);
		}
		else if (data && data->GetClientObject() == NULL) {
			printf("This session is already Expired. Overlapped: %X]\n", data);
		}
		else if (data && 
				data->GetClientObject() != NULL && 
				cbTransferred > 0) {
            
			std::shared_ptr<TcpClient> clientSession = data->GetClientObject();

			if (clientSession->GetRecvOverlapped() == data &&
				clientSession->GetRecvIoState() == IO_PENDING) {

                // TODO: 나중에 델리게이트 처리
				clientSession->OnReceived(cbTransferred);

				if (clientSession->IsValid()) {
					if (clientSession->RecvAsync() != 0) {
						err_display("WSARecv() Error at WorkerThread.");
					}
				}
				else {
					printf("RecvContext: Close Client invalid [Client: %X, Overlapped: %X]\n", data->GetClientObject(), data);

					CloseSession(data);
				}
			}

			if (clientSession->GetSendOverlapped() == data &&
				clientSession->GetSendIoState() == IO_PENDING) {
				
				clientSession->OnSend(cbTransferred);

				if (clientSession->IsValid()) {
                    if (clientSession->FlushSendBuffer() != 0) {
						err_display("WSASend() Error at WorkerThread.");
					}
				}
				else {
					printf("SendContext: Close Client invalid [Client: %X, Overlapped: %X]\n", data->GetClientObject(), data);

					CloseSession(data);
				}
			}
		}
		else {
            printf("Unhandled case is occurred.\n");
            if (data != NULL) {
                CloseSession(data);
            }
		}
	}

	return 0;
}



///



Network::Network(void) {
}

Network::~Network(void) {
}

void Network::Initialize(void) {
	if (!StartNetwork())
	{
		// log here.
		return;
	}

	// MEMO: pre-allocation
	g_recvIoPool.GrowPoolSize(2048);
	g_sendIoPool.GrowPoolSize(2048);

	// create I/O completion port
	const int numberOfThread = 4;   //TODO: remove magic number later.
	m_Iocp.StartIocpThread(numberOfThread);

	// create listen socket.
	m_ListenSocket.Bind(9000);
}

void Network::RunWithAcceptEx(void) {
    /*
    LPPER_HANDLE_DATA data = new PER_HANDLE_DATA();
    ZeroMemory(&(data->Overlapped), sizeof(data->Overlapped));
    m_Iocp.BindSocket((HANDLE)(m_ListenSocket.GetSocket()), (DWORD)(&m_ListenSocket));
    data->Socket.Initialize();

    LPPER_HANDLE_DATA data2 = new PER_HANDLE_DATA();
    ZeroMemory(&(data2->Overlapped), sizeof(data2->Overlapped));
    m_Iocp.BindSocket((HANDLE)(m_ListenSocket.GetSocket()), (DWORD)(&m_ListenSocket));
    data2->Socket.Initialize();
    
    BOOL result = m_ListenSocket.AcceptEx(data->Socket.GetSocket(),
        NULL, 0, 0, 0, &(data->Overlapped));
    if (result == FALSE) {
        err_display("fail with acceptEx");
    }

    BOOL result2 = m_ListenSocket.AcceptEx(data2->Socket.GetSocket(),
        NULL, 0, 0, 0, &(data2->Overlapped));
    if (result2 == FALSE) {
        err_display("fail with acceptEx");
    }

    */
    while (true) {
        ::Sleep(1000);
    }
}

void Network::Run(void) {

	while (true) {
		// accept connection 
		SOCKADDR_IN saRemote;
		int remoteLen = sizeof(saRemote);
		SOCKET Accepted = m_ListenSocket.Accept(&saRemote, &remoteLen);

		// LPPER_HANDLE_DATA perHandleData = MakeClientSession(Accepted, saRemote, remoteLen);
		OverlappedIO* handleData = MakeClientSessionEx(Accepted, saRemote, remoteLen);
		m_Iocp.BindSocket((HANDLE)(Accepted), static_cast<DWORD>(Accepted));

		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(saRemote.sin_addr), ntohs(saRemote.sin_port));
		printf("Socket number %d connected\n", Accepted);

		if (handleData->GetClientObject()->RecvAsync() != 0) {
			err_display("RecvAsync() Error at main thread");
		}
	}
}

void Network::Cleanup() {
	EndNetwork();
} 

bool Network::StartNetwork() {
	static WSADATA wsa;
	int result = ::WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result == 0)
	{
		return true;
	}
	// TODO: log here. 
	return false;
}

void Network::EndNetwork() {
	::WSACleanup();
}
