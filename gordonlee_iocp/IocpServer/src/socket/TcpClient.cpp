
#include "./TcpClient.h"

#include "utility/buffer.h"

TcpClient::TcpClient(void) 
: m_Socket(0)
, m_IsSetupAddr(false)
, m_pRecvBuffer(NULL)
, m_pSendBuffer(NULL)
, m_RecvBytes(0)
, m_SendBytes(0) {
    
    ::memset(&m_SocketAddr, 0, sizeof(m_SocketAddr));
    CreateBuffers();
}

TcpClient::~TcpClient(void) {
    RemoveBuffers();
}

int TcpClient::Initialize(void) {
    m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return 0;
}

int TcpClient::Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen) {
    m_Socket = _socket;
    ::memcpy(&(m_SocketAddr), &_addr, _addrLen);

    int addrlen = sizeof(m_SocketAddr);
    getpeername(m_Socket, (SOCKADDR *)&m_SocketAddr, &addrlen);
    m_IsSetupAddr = true;
    return 0;
}

int TcpClient::SendAsync(void) {
	return 0;
}

int TcpClient::Send(byte* _buffer, int _sendBytes) {
    m_IoState = IO_SENDING;

    int writtenBytes = m_pSendBuffer->Write(_buffer, _sendBytes);
    if (writtenBytes != _sendBytes) {
        return -1;
    }
    
    return ::send(m_Socket, m_pSendBuffer->GetPtr(), m_pSendBuffer->GetLength(), 0);
}

int TcpClient::Send(Buffer* _buffer, int _sendBytes) {
    m_IoState = IO_SENDING;
	if (_buffer && _sendBytes > 0) {
        return ::send(m_Socket, _buffer->GetPtr(), _sendBytes, 0);
	}
	return -1;
}

int TcpClient::RecvAsync(const LPOVERLAPPED _overlapped) {
    m_IoState = IO_READING;

	DWORD recvBytes;
	DWORD flags = 0;
	
    WSABUF wsaBuf;
	wsaBuf.buf = m_pRecvBuffer->GetPtr();
	wsaBuf.len = BUFFER_SIZE;

	int result = ::WSARecv(
		m_Socket,
		&wsaBuf,
		1,
		&recvBytes,
		&flags,
		_overlapped,
		NULL);

	if (result == SOCKET_ERROR){
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			return -1;
		}
	}
	return 0;
}

void TcpClient::Close(bool isForce) {
	::closesocket(m_Socket);
}

const SOCKET TcpClient::GetSocket() const {
	return m_Socket;
}

const Buffer* TcpClient::GetRecvBuffer() {
    return m_pRecvBuffer;
}

const Buffer* TcpClient::GetSendBuffer() {
    return m_pSendBuffer;
}

const IO_STATE TcpClient::GetIoState() const {
    return m_IoState;
}
//// handling events
// FIXME: packet process flow
const int packet_header_length = 2; 
void TcpClient::OnReceived(unsigned long transferred) {
    m_RecvBytes += transferred;
    
    printf("[%s:%d] RecvData : %s\n",
        inet_ntoa(m_SocketAddr.sin_addr),
        ntohs(m_SocketAddr.sin_port),
        m_pRecvBuffer->GetPtr());

    // process pakcet
    int transferredSendData = Send(reinterpret_cast<byte*>(m_pRecvBuffer->GetPtr()), m_RecvBytes);
    if (transferredSendData < 0) {
        Close(true);
        return;
    }
    OnSend(transferredSendData);

    // TODO: 아래 두 변수 가능하면 나중에 묶을 것.
    m_pRecvBuffer->Clear();
    m_RecvBytes = 0;

    m_IoState = IO_CONNECTED;
}

void TcpClient::OnSend(unsigned long transferred) {
    m_IoState = IO_CONNECTED;
    
    m_pSendBuffer->Clear();
    m_RecvBytes = 0;
}


//// private functions
void TcpClient::CreateBuffers(void) {
    if (m_pRecvBuffer || m_pSendBuffer) {
        RemoveBuffers();
    }

    // TODO: error handling, when allocation would be failed.
    m_pRecvBuffer = new Buffer();
    m_pSendBuffer = new Buffer();
}

void TcpClient::RemoveBuffers(void) {
    if (m_pRecvBuffer) {
        m_pRecvBuffer->Clear();
        delete m_pRecvBuffer;
        m_pRecvBuffer = NULL;
    }

    if (m_pSendBuffer) {
        m_pSendBuffer->Clear();
        delete m_pSendBuffer;
        m_pSendBuffer = NULL;
    }
}