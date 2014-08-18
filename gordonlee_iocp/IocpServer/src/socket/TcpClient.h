// Copyright 2014 GordonLee
#pragma once

#include "common_header/base_header.h"
#include "iocp/iocp_structure.h"


enum IO_STATE {
    IO_NOT_CONNECTED = 0,
    IO_CONNECTED,
    IO_PENDING,
};

struct Packet;
class IBuffer;
class ILock;

class TcpClient {
public:
	TcpClient(void);
	virtual ~TcpClient(void);

    int Initialize(void);
	int Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen);

	void BindRecvOverlapped(OverlappedIO* _overlapped);
	void BindSendOverlapped(OverlappedIO* _overlapped);
	
	bool IsValid(void) const;

	int SendAsync();
    int Send(byte* _buffer, int _sendBytes);
    int EnqueueSendBuffer(byte* _buffer, int _sendBytes);
    int FlushSendBuffer();
	int Send(IBuffer* _buffer, int _sendBytes);

	int RecvAsync(void);
	void TryProcessPacket();
	
	void Close(bool isForce);

public:
    const SOCKET GetSocket() const ;
	const IBuffer* GetRecvBuffer();
    const IO_STATE GetRecvIoState() const;
	const IO_STATE GetSendIoState() const;
	OverlappedIO* GetRecvOverlapped();
	OverlappedIO* GetSendOverlapped();


    // handling events
 public:
     void OnReceived(unsigned long transferred);
     void OnSend(unsigned long transferred);

 private:
     bool CreateBuffers(void);
     void RemoveBuffers(void);
	 void ProcessPacket(Packet* packet);

 private:
	SOCKET m_Socket;
	SOCKADDR_IN m_SocketAddr;
	bool m_IsSetupAddr;
    bool m_isClosing;

    // MEMO: input from async WorkerThread, output to OnReceived events
	OverlappedIO* m_pRecvOverlapped;
	IO_STATE m_RecvIoState;
	IBuffer* m_pRecvBuffer;
    int m_RecvBytes;

	// MEMO: send operation needs thread safe mode.
	OverlappedIO* m_pSendOverlapped;
	IO_STATE m_SendIoState;
	IBuffer* m_pSendBuffer;
	int m_SendBytes;
	ILock* m_pSendLock;
	
};
