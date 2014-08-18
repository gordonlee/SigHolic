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
 // MEMO: User Interface
 public:
     TcpClient(void);
     virtual ~TcpClient(void);

     // send buffer에 패킷을 넣는다. 
     int WriteSendBuffer(const Packet* _packet);

     // send buffer에 있는 내용을 보낸다.
     int FlushSendBuffer();

     // send한 결과를 받아온다. 
     virtual void OnSend(int _errorCode);

 public:
    int Initialize(void);
	int Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen);

	void BindRecvOverlapped(OverlappedIO* _overlapped);
	void BindSendOverlapped(OverlappedIO* _overlapped);
	
	bool IsValid(void) const;


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

     int EnqueueSendBuffer(byte* _buffer, int _sendBytes);
     int SendAsync();

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
