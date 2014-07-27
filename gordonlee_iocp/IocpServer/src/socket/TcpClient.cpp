
#include "./TcpClient.h"

#include "utility/buffer.h"
#include "packet/Packet.h"

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

int TcpClient::Send(IBuffer* _buffer, int _sendBytes) {
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
	wsaBuf.buf = m_pRecvBuffer->GetEmptyPtr();
	wsaBuf.len = m_pRecvBuffer->GetEmptyLength();

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

const IBuffer* TcpClient::GetRecvBuffer() {
    return m_pRecvBuffer;
}

const IBuffer* TcpClient::GetSendBuffer() {
    return m_pSendBuffer;
}

const IO_STATE TcpClient::GetIoState() const {
    return m_IoState;
}

//// handling events
const int packet_length_header = 2; 
void TcpClient::OnReceived(unsigned long transferred) {
    m_RecvBytes += transferred;
	m_pRecvBuffer->ForceAddLength(transferred);
	
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
	
	if (m_RecvBytes <= 0) {
		// unexpected value. do close
		Close(true);
		return;
	}
	
	if (m_RecvBytes > 0 && m_RecvBytes < packet_length_header) {
		// TODO: try recv again
		m_IoState = IO_CONNECTED;
		return;
	}
	
	// process packet
	Packet* packetPointer = reinterpret_cast<Packet *>(m_pRecvBuffer->GetPtr());

	while (packetPointer->dataSize_ > 0 &&
		m_RecvBytes >= packetPointer->dataSize_ + packet_header_size)
	{
		byte packet[BUFFER_SIZE] = { 0, };
		::memset(packet, 0, BUFFER_SIZE);
		m_pRecvBuffer->Read(packetPointer->dataSize_ + packet_header_size, (char*)packet, BUFFER_SIZE);
		ProcessPacket((Packet*)packet);

		// FIXME: m_RecvBytes �� ������ �������� ���̽��� �ִ� �� ����.
		if (packetPointer->dataSize_ + packet_header_size > m_RecvBytes)
		{
			// �����̻����� �����Ͽ� ������.
			Close(true);
			return;
		}
		m_RecvBytes -= (packetPointer->dataSize_ + packet_header_size);

		if (m_pRecvBuffer->GetLength() > packet_length_header) {
			packetPointer = (Packet*)m_pRecvBuffer->GetPtr();
		}
		else {
			break;
		}
	}
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
		*/
		printf("[%s:%d] ProcessPacket [%d:%d:%d][%d:%d:%d]\n",
			inet_ntoa(m_SocketAddr.sin_addr),
			ntohs(m_SocketAddr.sin_port),
			packet->dataSize_,
			packet->flag_,
			packet->checkSum_,
			dataSection->packetId_,
			dataSection->clientId_,
			dataSection->sentTime_);
		
		int transferredSendData = Send(reinterpret_cast<byte*>(packet), packet->dataSize_ + 4);
		if (transferredSendData < 0) {
			Close(true);
			return;
		}
		OnSend(transferredSendData);
	}
	else if (packet->flag_ == 0x02) { // send as broadcast to all sessions

	}
	else {
		// Unknown flag
		Close(true);
	}
}

void TcpClient::OnSend(unsigned long transferred) {
    m_IoState = IO_CONNECTED;
    
    m_pSendBuffer->Clear();
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