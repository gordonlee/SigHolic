#include "tcp_session_pool.h"

#include "utility\scoped_lock.h"
#include "network\tcp_client.h"

TcpSessionPool::TcpSessionPool(ILock* _lock) 
: num_init_ccu_(100)
, lock_(_lock) {

}

TcpSessionPool::~TcpSessionPool(void) {

}

void TcpSessionPool::PushWaitQueue(TcpClient* _client) {
    AutoLock autolock(lock_);
    wait_clients_.push(_client);
}

TcpClient* TcpSessionPool::PopFromWaitQueue(void) {
    AutoLock autolock(lock_);
    if (wait_clients_.size() <= 0) {
        GrowWaitPoolSize();
    }

    TcpClient* dequeue = wait_clients_.front();
    wait_clients_.pop();

    return dequeue;
}

const bool TcpSessionPool::CheckWaitPoolSize() const {
    AutoLock autolock(lock_);

    // TODO: remove warning!
    return (wait_clients_.size() < get_num_init_ccu());
}

bool TcpSessionPool::GrowWaitPoolSize(void) {
    AutoLock autolock(lock_);

    TcpClient* alloc_data = new TcpClient[get_num_init_ccu()];
    data_pool_.push_back(alloc_data);

    TcpClient* push_back_ptr = alloc_data;
    for (int i = 0; i < get_num_init_ccu(); ++i) {
        PushWaitQueue(push_back_ptr);
        ++push_back_ptr;
    }
    return true;
}