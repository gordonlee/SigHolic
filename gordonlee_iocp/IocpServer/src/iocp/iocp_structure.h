#pragma once

// MEMO: c++ 11 std header
#include <list>
#include <queue>

#include "common_header/base_header.h"
#include "utility/AutoLock.h"

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

class OverlappedIoPool {
 public:
	 OverlappedIoPool(void) {
	 }

	 ~OverlappedIoPool(void) {
	 }

	 OverlappedIO* Dequeue() {
		 return NULL;
	 }

	 void Enqueue(const OverlappedIO* _overlapped) {

	 }

	 void Clear(void) {
		 for (auto dataPtr : m_dataPool) {
			 if (dataPtr != NULL) {
				 delete [] dataPtr;
				 dataPtr = NULL;
			 }
		 }

		 m_dataPool.clear();

		 // TODO: 사용중인 애들이 있는 상태에서 클리어 되면 어떻게 할까?
		 // TODO: clear waiting queue by c++11 methods.
		 // m_waitingQueue.
	 }

 private:	// functions
	 BOOL GrowPoolSize(int _elementCount) {
		 
		 if (_elementCount <= 0) { return FALSE; }

		 OverlappedIO* allocArray = new OverlappedIO[_elementCount];
		 OverlappedIO* insertPtr = allocArray;
		 for (int i = 0; i < _elementCount; ++i) {
			 m_waitingQueue.push(insertPtr);
			 insertPtr += sizeof(OverlappedIO);
		 }
		 m_dataPool.push_back(allocArray);
		 return TRUE;
	 }

 private:	// members
	 std::queue<OverlappedIO*> m_waitingQueue;
	 std::list<OverlappedIO*> m_dataPool;
	 
};
