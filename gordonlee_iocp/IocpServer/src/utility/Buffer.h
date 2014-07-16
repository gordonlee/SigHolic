// Copyright 2014 Gordonlee
#pragma once 

#include "common_header/base_header.h"
// FIXME: remove stdio.h
#include <stdio.h>

// FIXME: Use interface later, so that buffer changes other concept.
class IBuffer {
};

const int BUFFER_SIZE = 1024;
// FIXME: Naming later.
class Buffer {
 public:
    Buffer(void): m_Length(0) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
    }
    char* GetPtr(void) {
        return reinterpret_cast<char*>(m_Buffer);
    }

    /*
    const byte* GetPtr(void) const {
        return m_Buffer;
    }
    */

    const int GetLength(void) const {
        return m_Length;
    }

    int Write(const byte* _startPoint, int _length) {
        if (_length > BUFFER_SIZE) {
            return -1;
        }

        ::memcpy(m_Buffer, _startPoint, _length);
        m_Length += _length;
        return m_Length;
    }

    void Clear(void) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
        m_Length = 0;
    }

 private:
    byte m_Buffer[BUFFER_SIZE];
    int m_Length;
};
