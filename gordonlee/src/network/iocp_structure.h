#pragma once 

#include "core_header\base_header.h"

class TcpClient;

typedef struct _OverlappedIO sealed : public OVERLAPPED {
 public:
     _OverlappedIO() : client_(NULL) {
		Reset();
	}

	void Reset() {
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
		Pointer = 0;
		hEvent = 0;
	}

    void set_session(std::shared_ptr<TcpClient> _client) {
        client_ = _client;
	}

    std::shared_ptr<TcpClient> client(void) {
        return client_;
	}

 private:
    std::shared_ptr<TcpClient> client_;
} OverlappedIO;