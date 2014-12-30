#pragma once 
#include <list>
#include <queue>
#include "core_header\base_header.h"

class TcpClient;
class ILock;
// TODO: template 으로 만들어 볼 것.
class TcpSessionPool {
 public:
     TcpSessionPool(ILock* _lock);
     virtual ~TcpSessionPool(void);

     void PushWaitQueue(TcpClient* _client);
     TcpClient* PopFromWaitQueue(void);
     const bool CheckWaitPoolSize() const;
     bool GrowWaitPoolSize(void);
     
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