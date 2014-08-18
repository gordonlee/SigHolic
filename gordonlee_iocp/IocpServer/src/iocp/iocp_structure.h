#pragma once

// MEMO: c++ 11 std header
#include <list>
#include <queue>
#include <memory>

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

	void SetClientObject(std::shared_ptr<TcpClient> _client) {
        Client = _client;
    }

    std::shared_ptr<TcpClient> GetClientObject(void) {
        return Client;
    }

private:
    std::shared_ptr<TcpClient> Client;
} OverlappedIO;


const static int g_default_io_pool_size = 2048;

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
             //MakeOverlappedIo();
             if (GrowPoolSize(g_default_io_pool_size) == FALSE) {
                 throw;
             }
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

	 BOOL GrowPoolSize(int _elementCount) {
		 AutoLock autolock(m_lock);

		 if (_elementCount <= 0) { return FALSE; }
		 OverlappedIO* allocArray = new OverlappedIO[_elementCount]();
		 m_dataPool.push_back(allocArray);

		 OverlappedIO* insertPtr = allocArray;
		 for (int i = 0; i < _elementCount; ++i) {
			 insertPtr->Reset();
			 // m_waitingQueue.push(insertPtr);
			 Enqueue(insertPtr);
			 // TODO: 공부 다시 하자 ㄱ-
			 ++insertPtr;
		 }
		 return TRUE;
	 }

 private:	// functions

     OverlappedIO* MakeOverlappedIo() {
         OverlappedIO* element = new OverlappedIO();
         m_waitingQueue.push(element);
         return element;
     }

	 

 private:	// members
	 std::queue<OverlappedIO*> m_waitingQueue;
	 std::list<OverlappedIO*> m_dataPool;
	 ILock* m_lock;
};
