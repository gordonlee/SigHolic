
#include "./TcpClient.h"
#include "./TcpSessionManager.h"

#include "packet/Packet.h"
#include "utility/buffer.h"
#include "utility/AutoLock.h"
#include "iocp/iocp_structure.h"

// TODO: 이걸 만들어 말어..
namespace {
	class IoContext {

	};
}

///

TcpClient::TcpClient(void) 
: m_Socket(0)
, m_IsSetupAddr(false)
, m_pRecvBuffer(NULL)
, m_pSendBuffer(NULL)
, m_RecvBytes(0)
, m_SendBytes(0)
, m_RecvIoState(IO_NOT_CONNECTED)
, m_SendIoState(IO_NOT_CONNECTED)
, m_isClosing(false)
, m_pRecvOverlapped(NULL)
, m_pSendOverlapped(NULL) {
	::memset(&m_SocketAddr, 0, sizeof(m_SocketAddr));

	m_pSendLock = new CriticalSectionLock();

    CreateBuffers();
}

TcpClient::~TcpClient(void) {
    RemoveBuffers();
	
	if (m_pSendLock) {
		delete m_pSendLock;
	}
}

int TcpClient::Initialize(void) {
    m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    m_RecvIoState = IO_CONNECTED;
    return 0;
}

int TcpClient::Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen) {
	m_Socket = _socket;
    ::memcpy(&(m_SocketAddr), &_addr, _addrLen);

    int addrlen = sizeof(m_SocketAddr);
    getpeername(m_Socket, (SOCKADDR *)&m_SocketAddr, &addrlen);
    m_IsSetupAddr = true;

	m_RecvIoState = IO_CONNECTED;

    return 0;
}

void TcpClient::BindRecvOverlapped(OverlappedIO* _overlapped) {
	m_pRecvOverlapped = _overlapped;
}

void TcpClient::BindSendOverlapped(OverlappedIO* _overlapped) {
	m_pSendOverlapped = _overlapped;
}

int TcpClient::SendAsync() {
	AutoLock autoLockInstance(m_pSendLock);
	
	if (m_pSendBuffer->GetLength() > 0) {
		if (m_SendIoState == IO_PENDING) {
			// printf("TcpClient::SendAsync> m_SendIoState is IO_PENDING\n");
		}
		else {
			m_SendIoState = IO_PENDING;

			DWORD sendBytes;
			DWORD flags = 0;

			WSABUF wsaBuf;
			wsaBuf.buf = m_pSendBuffer->GetPtr();
			wsaBuf.len = m_pSendBuffer->GetLength();

			m_pSendOverlapped->Reset();
			// call WSASend()
			int result = ::WSASend(
				m_Socket,
				&wsaBuf,
				1,
				&sendBytes,
				flags,
				m_pSendOverlapped,
				NULL);

			if (result == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					return -1;
				}
			}
		}
	}
	else {
		m_SendIoState = IO_CONNECTED;
	}
	
	return 0;
}

int TcpClient::Send(byte* _buffer, int _sendBytes) {
	AutoLock autoLockInstance(m_pSendLock);

	m_SendIoState = IO_PENDING;

    int writtenBytes = m_pSendBuffer->Write(_buffer, _sendBytes);
    if (writtenBytes != _sendBytes) {
        return -1;
    }
    
    return ::send(m_Socket, m_pSendBuffer->GetPtr(), m_pSendBuffer->GetLength(), 0);
}

int TcpClient::EnqueueSendBuffer(byte* _buffer, int _sendBytes) {
    AutoLock autoLockInstance(m_pSendLock);

    // m_SendIoState = IO_PENDING;

    int writtenBytes = m_pSendBuffer->Write(_buffer, _sendBytes);
    if (writtenBytes != _sendBytes) {
        return -1;
    }
    return writtenBytes;
}

/*
int TcpClient::FlushSendBuffer() {
    AutoLock autoLockInstance(m_pSendLock);

    if (m_pSendBuffer->GetLength() > 0) {
        return ::send(m_Socket, m_pSendBuffer->GetPtr(), m_pSendBuffer->GetLength(), 0);
    }
    return 0;
}
*/

int TcpClient::FlushSendBuffer() {
	AutoLock autoLockInstance(m_pSendLock);

	if (m_pSendBuffer->GetLength() > 0) {
		return SendAsync();
	}
	return 0;
}

int TcpClient::Send(IBuffer* _buffer, int _sendBytes) {
	AutoLock autoLockInstance(m_pSendLock);

	m_SendIoState = IO_PENDING;
	if (_buffer && _sendBytes > 0) {
        return ::send(m_Socket, _buffer->GetPtr(), _sendBytes, 0);
	}
	return -1;
}

int TcpClient::RecvAsync(void) {
    //printf("StartPending [Client: %X, Overlapped: %X]\n", this, m_pRecvOverlapped);

    if (GetRecvIoState() != IO_CONNECTED) {
        printf("RecvAsync.m_RecvIoState is not IO_CONNECTED. Check threading synchronization.\n");
    }

    m_RecvIoState = IO_PENDING;

	DWORD recvBytes;
	DWORD flags = 0;

	WSABUF wsaBuf;
	wsaBuf.buf = m_pRecvBuffer->GetEmptyPtr();
	wsaBuf.len = m_pRecvBuffer->GetEmptyLength();

	m_pRecvOverlapped->Reset();
    
    int result = ::WSARecv(
		m_Socket,
		&wsaBuf,
		1,
		&recvBytes,
		&flags,
		m_pRecvOverlapped,
		NULL);

	if (result == SOCKET_ERROR){
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			return -1;
		}
	}
	return 0;
}

void TcpClient::Close(bool isForce) {
    //RemoveBuffers();
	::closesocket(m_Socket);
    m_isClosing = true;
}

const SOCKET TcpClient::GetSocket() const {
	return m_Socket;
}

const IBuffer* TcpClient::GetRecvBuffer() {
    return m_pRecvBuffer;
}

const IO_STATE TcpClient::GetRecvIoState() const {
    return m_RecvIoState;
}

const IO_STATE TcpClient::GetSendIoState() const {
	return m_SendIoState;
}

OverlappedIO* TcpClient::GetRecvOverlapped() {
	return m_pRecvOverlapped;
}

OverlappedIO* TcpClient::GetSendOverlapped() {
	return m_pSendOverlapped;
}


//// handling events
const int packet_length_header = 2; 
void TcpClient::OnReceived(unsigned long transferred) {
    m_RecvBytes += transferred;
	m_pRecvBuffer->ForceAddLength(transferred);
    
    m_RecvIoState = IO_CONNECTED;

	/*
    printf("[%s:%d] RecvData : %s\n",
        inet_ntoa(m_SocketAddr.sin_addr),
        ntohs(m_SocketAddr.sin_port),
        m_pRecvBuffer->GetPtr());
	*/

	TryProcessPacket();
}

const int packet_header_size = sizeof(PacketHeader);

void TcpClient::TryProcessPacket() {
	if (m_pRecvBuffer->GetLength() < packet_length_header) {
		return;
	}
	
	// process packet
	Packet* packetPointer = reinterpret_cast<Packet *>(m_pRecvBuffer->GetPtr());

	while (packetPointer->dataSize_ > 0 &&
        m_pRecvBuffer->GetLength() >= packetPointer->dataSize_ + packet_header_size)
	{
		byte packet[BUFFER_SIZE] = { 0, };
		::memset(packet, 0, BUFFER_SIZE);

        int readBytes = m_pRecvBuffer->Read(packetPointer->dataSize_ + packet_header_size, (char*)packet, BUFFER_SIZE);

        m_RecvBytes -= readBytes;

		ProcessPacket((Packet*)packet);

        if (m_pRecvBuffer != NULL && 
            m_pRecvBuffer->GetLength() > packet_length_header) {
            Packet* packetPointer = reinterpret_cast<Packet *>(m_pRecvBuffer->GetPtr());
		}
		else {
			break;
		}
	}
    
	FlushSendBuffer();
}

void TcpClient::ProcessPacket(Packet* packet) {

	/// Verify CheckSum
	if (packet->checkSum_ == 0x55) {

	}
	else {
		// Unknown checksum
		Close(true);
	}

	/// Verify Flag
	if (packet->flag_ == 0x01) {	// send as echo
		PacketData* dataSection = (PacketData*)packet->data_;
        /*
		printf("[%s:%d] ProcessPacket [%d:%d:%d][%d:%d:%d][%s]\n",
			inet_ntoa(m_SocketAddr.sin_addr),
			ntohs(m_SocketAddr.sin_port),
			packet->dataSize_,
			packet->flag_, 
			packet->checkSum_,
			dataSection->packetId_,
			dataSection->clientId_,
			dataSection->sentTime_,
			dataSection->data_);
        
		printf("[%s:%d] ProcessPacket [%d:%d:%d][%d:%d:%d]\n",
			inet_ntoa(m_SocketAddr.sin_addr),
			ntohs(m_SocketAddr.sin_port),
			packet->dataSize_,
			packet->flag_,
			packet->checkSum_,
			dataSection->packetId_,
			dataSection->clientId_,
			dataSection->sentTime_);
		*/
        /*
		int transferredSendData = Send(
			reinterpret_cast<byte*>(packet), 
			packet->dataSize_ + packet_header_size);
            */
        int transferredSendData = EnqueueSendBuffer(
            reinterpret_cast<byte*>(packet),
            packet->dataSize_ + packet_header_size);
		if (transferredSendData < 0) {
            if (transferredSendData == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
            }
			Close(true);
			return;
		}
		// OnSend(transferredSendData);
	}
	else if (packet->flag_ == 0x02) { // send as broadcast to all sessions
		for (auto& client : TcpSessionManager.GetWholeClients()) {
			if (client != NULL && client->IsValid()) {
                int transferredSendData = client->EnqueueSendBuffer(
                    reinterpret_cast<byte*>(packet),
                    packet->dataSize_ + packet_header_size);
                if (transferredSendData < 0) {
                    if (transferredSendData == SOCKET_ERROR) {
                        printf("send failed with error: %d\n", WSAGetLastError());
                    }
                    Close(true);
                    return;
                }

                client->FlushSendBuffer();
			}
		}
	}
	else {
		// Unknown flag
		Close(true);
	}
}

bool TcpClient::IsValid(void) const {
    AutoLock autoLockInstance(m_pSendLock);

    // TODO: varify valid
    if (m_isClosing) {
        return false;
    }
	return true;
}

void TcpClient::OnSend(unsigned long transferred) {
	AutoLock autoLockInstance(m_pSendLock);

	m_pSendBuffer->ThrowAway(transferred);

    m_SendIoState = IO_CONNECTED;
}


//// private functions
bool TcpClient::CreateBuffers(void) {
    if (m_pRecvBuffer || m_pSendBuffer) {
        RemoveBuffers();
    }

	m_pRecvBuffer = BufferFactory::CreateBuffer(PULLED_BUFFER);
	if (m_pRecvBuffer == NULL) {
		return false;
	}
	m_pSendBuffer = BufferFactory::CreateBuffer(PULLED_BUFFER);
	if (m_pSendBuffer == NULL) {
		return false;
	}

	return true;
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