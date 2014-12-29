#pragma once 
#include <list>
#include <queue>

#include "core_header\base_header.h"
#include "utility\scoped_lock.h"
#include "network\tcp_client.h"


// TODO: template 으로 만들어 볼 것.
class TcpSessionPool {
 public:
     TcpSessionPool(ILock* _lock) : num_init_ccu_(100), lock_(_lock) {

     }

     ~TcpSessionPool(void) {

     }

     void PushWaitQueue(TcpClient* _client) {
         AutoLock autolock(lock_);
         wait_clients_.push(_client);
     }

     TcpClient* PopFromWaitQueue(void) {
         AutoLock autolock(lock_);
         if (wait_clients_.size() <= 0) {
             GrowWaitPoolSize();
         }

         TcpClient* dequeue = wait_clients_.front();
         wait_clients_.pop();

         return dequeue;
     }

     const bool CheckWaitPoolSize() const {
         AutoLock autolock(lock_);

         // TODO: remove warning!
         return (wait_clients_.size() < get_num_init_ccu());
     }

     bool GrowWaitPoolSize(void) {
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

     
 public:
     const int get_num_init_ccu(void) const {
         return num_init_ccu_;
     }

 private:
     const int num_init_ccu_;
     std::queue<TcpClient*> wait_clients_;
     std::list<TcpClient*> data_pool_; // array data ( TcpClient[num_init_ccu_] )
     ILock* lock_;
     
};