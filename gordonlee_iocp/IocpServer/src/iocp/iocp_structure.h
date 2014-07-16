#pragma once

#include "common_header/base_header.h"


class TcpClient;
typedef struct _OverlappedIO sealed : public OVERLAPPED {
    _OverlappedIO() {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        Pointer = 0;
        hEvent = 0;
    }
    TcpClient* Client;
} OverlappedIO;
