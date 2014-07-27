// Copyright 2014 Gordonlee
#pragma once 

#include "common_header/base_header.h"
// FIXME: remove stdio.h
#include <stdio.h>

// FIXME: Use interface later, so that buffer changes other concept.
class IBuffer {
};

const int BUFFER_SIZE = 65536;

// FIXME: Naming later.
class Buffer {
 public:
    Buffer(void): m_Length(0) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
    }

    char* GetPtr(void) {
        return reinterpret_cast<char*>(m_Buffer);
    }

	const int GetLength(void) const {
		return m_Length;
	}

	char* GetEmptyPtr(void) {
		return reinterpret_cast<char*>(m_Buffer + m_Length);
	}

	const int GetEmptyLength(void) {
		return BUFFER_SIZE - m_Length;
	}

    int Write(const byte* _startPoint, int _length) {
        if (_length > BUFFER_SIZE) {
            return -1;
        }

        ::memcpy(m_Buffer, _startPoint, _length);
        m_Length += _length;
        return m_Length;
    }

	void ForceAddLength(int _length) {
		m_Length += _length;
	}

	int Read(int _readLength, char* _destBuffer, int _destLength) {
		if (_readLength > m_Length) {
			// FIXME: ��� TcpClient::m_RecvBytes�� m_Length�� �ٸ���??? -> ���ú꿡�� ������ �������־���
			throw;
		}

		errno_t copyResult = ::memcpy_s(_destBuffer, _destLength, m_Buffer, _readLength);
		// TODO: check copyResult
		m_Length -= _readLength;

		// pull the buffer in front of start pointer
		if (m_Length > 0) {
			byte tempBuffer[BUFFER_SIZE] = { 0, };
			int tempLength = m_Length;
			::memcpy_s(tempBuffer, BUFFER_SIZE, m_Buffer + _readLength, m_Length);

			Clear();

			Write(tempBuffer, tempLength);
		}
		return _readLength;
	}

    void Clear(void) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
        m_Length = 0;
    }

 private:
    byte m_Buffer[BUFFER_SIZE];
    int m_Length;
};
