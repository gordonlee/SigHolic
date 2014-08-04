// Copyright 2014 Gordonlee
#pragma once 

#include "common_header/base_header.h"
// FIXME: remove stdio.h
#include <stdio.h>

// FIXME: Use interface later, so that buffer changes other concept.
class IBuffer {
 public:
	virtual char* GetPtr(void) = 0;
	virtual const int GetLength(void) const = 0;
	virtual char* GetEmptyPtr(void) = 0;
	virtual const int GetEmptyLength(void) = 0;
	virtual int Write(const byte* _startPoint, int _length) = 0;
	virtual void ForceAddLength(int _length) = 0;
	virtual int Read(const int _readLength, char* _destBuffer, int _destLength) = 0;
	virtual void Clear(void) = 0;
};

const int BUFFER_SIZE = 65536;

// FIXME: Naming later.
class PulledBuffer : public IBuffer {
 public:
	 PulledBuffer(void) : m_Length(0) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
    }

	 virtual ~PulledBuffer(void) {
	 }

	virtual char* GetPtr(void) {
        return reinterpret_cast<char*>(m_Buffer);
    }

	virtual const int GetLength(void) const {
		return m_Length;
	}

	virtual char* GetEmptyPtr(void) {
		return reinterpret_cast<char*>(m_Buffer + m_Length);
	}

	virtual const int GetEmptyLength(void) {
		return BUFFER_SIZE - m_Length;
	}

	virtual int Write(const byte* _startPoint, int _length) {
        if (m_Length + _length > BUFFER_SIZE) {
            return -1;
        }

        ::memcpy_s(m_Buffer + m_Length, BUFFER_SIZE - m_Length, _startPoint, _length);
        m_Length += _length;
        return _length;
    }

	virtual void ForceAddLength(int _length) {
		m_Length += _length;
	}

	virtual int Read(const int _readLength, char* _destBuffer, int _destLength) {
		if (_readLength > m_Length) {
			// MEMO: 어떻게 TcpClient::m_RecvBytes랑 m_Length랑 다르지??? 
            // -> 리시브에서 받은거 날리고있었네
			throw;
		}

        int cacheLength = _readLength;

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

	virtual void Clear(void) {
        ::memset(m_Buffer, 0, BUFFER_SIZE);
        m_Length = 0;
    }

 private:
    byte m_Buffer[BUFFER_SIZE];
    int m_Length;
};


//// factory

enum BUFFER_TYPE {
	PULLED_BUFFER = 0,
	// CIRCULAR_BUFFER,
};

class BufferFactory {
 public:
	 static IBuffer* CreateBuffer(BUFFER_TYPE _type) {
		 
		 // TODO: error handling, when allocation would be failed.

		 if (_type == PULLED_BUFFER) {
			 return new PulledBuffer();
		 }
		 else {
			 return NULL;
		 }
	 }
};
