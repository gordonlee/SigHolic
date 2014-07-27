// Copyright 2014 GordonLee
#pragma once

#include "common_header/base_header.h"

enum IO_STATE {
    IO_NOT_CONNECTED = 0,
    IO_CONNECTED,
    IO_READING,
    IO_SENDING,
};

struct Packet;
class Buffer;

class TcpClient {
public:
	TcpClient(void);
	virtual ~TcpClient(void);

    int Initialize(void);
	int Initialize(const SOCKET _socket, const SOCKADDR_IN& _addr, const int _addrLen);
	
	int SendAsync(void);
    int Send(byte* _buffer, int _sendBytes);
	int Send(Buffer* _buffer, int _sendBytes);

	int RecvAsync(const LPOVERLAPPED _overlapped);
	void TryProcessPacket();
	
	void Close(bool isForce);

public:
    const SOCKET GetSocket() const ;
    const Buffer* GetRecvBuffer();
    const Buffer* GetSendBuffer();
    const IO_STATE GetIoState() const ;

    // handling events
 public:
     void OnReceived(unsigned long transferred);
     void OnSend(unsigned long transferred);

 private:
     void CreateBuffers(void);
     void RemoveBuffers(void);
	 void ProcessPacket(Packet* packet);

 private:
	SOCKET m_Socket;
	SOCKADDR_IN m_SocketAddr;
	bool m_IsSetupAddr;
    IO_STATE m_IoState;
    
    // MEMO: input from async WorkerThread, output to OnReceived events
    Buffer* m_pRecvBuffer;
    Buffer* m_pSendBuffer;

    unsigned long m_RecvBytes;
    unsigned long m_SendBytes;
};
