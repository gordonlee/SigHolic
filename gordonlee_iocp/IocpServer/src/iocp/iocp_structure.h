#pragma once

// MEMO: c++ 11 std header
#include <list>
#include <queue>

#include "common_header/base_header.h"
#include "utility/AutoLock.h"

class TcpClient;
typedef struct _OverlappedIO sealed : public OVERLAPPED {
	
    _OverlappedIO() : Client(NULL) {
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

    void SetClientObject(TcpClient* _client) {
        Client = _client;
    }

    TcpClient* GetClientObject(void) {
        return Client;
    }

private:
    TcpClient* Client;
} OverlappedIO;


const int g_default_io_pool_size = 4;

class OverlappedIoPool {
 public:
	 OverlappedIoPool(ILock* _lock) : m_lock(_lock){
	 }

	 ~OverlappedIoPool(void) {
		 Clear();
	 }

	 OverlappedIO* Dequeue() {
		 AutoLock autolock(m_lock);

		 if (m_waitingQueue.size() == 0) {
             MakeOverlappedIo();
             /*
             if (GrowPoolSize(g_default_io_pool_size) == FALSE) {
                 throw;
             }
             */
		 }
		 OverlappedIO* dequeue = m_waitingQueue.front();
		 m_waitingQueue.pop();
		 return dequeue;
	 }

	 void Enqueue(OverlappedIO* _overlapped) {
		 AutoLock autolock(m_lock);

		 m_waitingQueue.push(_overlapped);
	 }

	 void Clear(void) {
		 // TODO: 사용중인 애들이 있는 상태에서 클리어 되면 어떻게 할까?

		 AutoLock autolock(m_lock);

		 for (auto& dataPtr : m_dataPool) {
			 if (dataPtr != NULL) {
				 delete [] dataPtr;
				 dataPtr = NULL;
			 }
		 }
		 m_dataPool.clear();

		 // clear queue
		 std::queue<OverlappedIO*> emptyQueue;
		 std::swap(emptyQueue, m_waitingQueue);
	 }

 private:	// functions

     OverlappedIO* MakeOverlappedIo() {
         OverlappedIO* element = new OverlappedIO();
         m_waitingQueue.push(element);
         return element;
     }

	 // FIXME: Test Grow again
	 BOOL GrowPoolSize(int _elementCount) {
		 AutoLock autolock(m_lock);

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
	 ILock* m_lock;
};
