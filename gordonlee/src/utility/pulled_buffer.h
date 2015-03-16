#pragma once

#include "core_header/base_header.h"
// FIXME: remove stdio.h
#include <stdio.h>

const int BUFFER_SIZE = 65536 * 4;

class PulledBuffer {
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

	// FIXME: 맘에 안든다. 바꾸고 싶다.
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

	virtual int ThrowAway(const int _throwLength) {
		if (_throwLength > m_Length) {
			throw;
		}

		m_Length -= _throwLength;
		// pull the buffer in front of start pointer
		if (m_Length > 0) {
			byte tempBuffer[BUFFER_SIZE] = { 0, };
			int tempLength = m_Length;
			::memcpy_s(tempBuffer, BUFFER_SIZE, m_Buffer + _throwLength, m_Length);

			Clear();

			Write(tempBuffer, tempLength);
		}

		return _throwLength;
	}

private:
	byte m_Buffer[BUFFER_SIZE];
	int m_Length;
};