
#include "network.h"

#include "socket/TcpSessionManager.h"

namespace {
	// FIXME: replace this function to log class
	void err_display(char *msg) {
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL);
		printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
}

unsigned int __stdcall WorkerThread(LPVOID lpParam) {

	// TODO: write down worker thread.
	HANDLE handle = (HANDLE)lpParam;
	int retval;
	DWORD cbTransferred;
	SOCKET client_socket = 0;
	LPPER_HANDLE_DATA data = NULL;

	while (TRUE) {
		retval = ::GetQueuedCompletionStatus(
			handle,
			&cbTransferred,
			(PULONG_PTR)&client_socket,
			(LPOVERLAPPED *)(&data),
			INFINITE);

		DWORD lastError = GetLastError();
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
			if (retval == 0) {
				DWORD temp1, temp2;
				if (data != NULL)
				{
					::WSAGetOverlappedResult(
						data->Socket.GetSocket(),
						&(data->Overlapped),
						&temp1,
						FALSE,
						&temp2);
				}

				err_display("WSAGetOverlappedResult()");
			}

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(data->Socket.GetSocket(), (SOCKADDR *)&clientaddr, &addrlen);

			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			data->Socket.Close(false);
			TcpSessionManager.RemoveTcpClient(&(data->Socket));
			//FIXME: delete data at here is dangerous!
			delete data;
			continue;
		}
		else if (data) {
            if (cbTransferred > 0 && data->Socket.GetRecvIoState() == IO_PENDING) {

                // TODO: 나중에 델리게이트 처리
                data->Socket.OnReceived(cbTransferred);
			}

			::ZeroMemory(&(data->Overlapped), sizeof(data->Overlapped));


            if (data->Socket.IsValid()) {
                if (data->Socket.RecvAsync(&(data->Overlapped)) != 0) {
                    err_display("WSARecv() Error at WorkerThread.");
                }
            }
            else {
                delete data;
                continue;
            }
            
		}
		else {
			//TODO: do something when it catches any case.
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

	// create I/O completion port
	const int numberOfThread = 4;   //TODO: remove magic number later.
	m_Iocp.StartIocpThread(numberOfThread);

	// create listen socket.
	m_ListenSocket.Bind(9000);
}

void Network::RunWithAcceptEx(void) {

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

		LPPER_HANDLE_DATA perHandleData = MakeClientSession(Accepted, saRemote, remoteLen);
		m_Iocp.BindSocket((HANDLE)(Accepted), static_cast<DWORD>(Accepted));

		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(saRemote.sin_addr), ntohs(saRemote.sin_port));
		printf("Socket number %d connected\n", Accepted);

		// start socket's i/o job
		// MEMO: 초기화 안된 Overlapped 쓰면 객체 뻑남.
		ZeroMemory(&(perHandleData->Overlapped), sizeof(perHandleData->Overlapped));

		if (perHandleData->Socket.RecvAsync(
			&(perHandleData->Overlapped)
			) != 0) {
			err_display("RecvAsync() Error at main thread");
		}
	}
}

LPPER_HANDLE_DATA Network::MakeClientSession(SOCKET _clientSocket, const SOCKADDR_IN& _addr, const int _addrLen) {
	LPPER_HANDLE_DATA data = new PER_HANDLE_DATA();
	data->Socket.Initialize(_clientSocket, _addr, _addrLen);
	TcpSessionManager.AddTcpClient(&(data->Socket));
	return data;
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
