// Copyright 2014 GordonLee
#pragma once

#include "common_header/base_header.h"

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
	
	bool IsValid(void) const;

	int SendAsync(void);
    int Send(byte* _buffer, int _sendBytes);
	int Send(IBuffer* _buffer, int _sendBytes);

	int RecvAsync(const LPOVERLAPPED _overlapped);
	void TryProcessPacket();
	
	void Close(bool isForce);

public:
    const SOCKET GetSocket() const ;
	const IBuffer* GetRecvBuffer();
    const IO_STATE GetRecvIoState() const ;

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

    // MEMO: input from async WorkerThread, output to OnReceived events
	IO_STATE m_RecvIoState;
	IBuffer* m_pRecvBuffer;
    unsigned long m_RecvBytes;

	// MEMO: send operation needs thread safe mode.
	IO_STATE m_SendIoState;
	IBuffer* m_pSendBuffer;
	unsigned long m_SendBytes;
	ILock* m_pSendLock;
	
	
};
