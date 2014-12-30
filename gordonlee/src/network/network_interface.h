#pragma once 
#include <thread>

#include "core_header\base_header.h"
#include "network\tcp_client.h"
#include "network\tcp_server.h"
#include "network\tcp_session_manager.h"
#include "network\tcp_session_pool.h"
#include "network\iocp_binder.h"
#include "utility\scoped_lock.h"


enum WSAVersion {
    VERSION_2_2 = 0,
};

enum IOModel {
    DEFAULT = 0,
    ASYNC_ACCPET,
};

// TODO: is this class really needed?
class INetwork {
public:
    virtual bool Initialize(IOModel _io_model) = 0;
    virtual void Run() = 0;
    virtual void Clean() = 0;
    virtual const HANDLE get_thread_handle() = 0;
};

class Network : public INetwork {
 public:
     Network() 
         : server_(NULL)
         , io_model_(IOModel::DEFAULT)
         , is_thread_running_(true)
         , num_init_ccu_(100) {
     }
     virtual ~Network() {
         accept_thread_.detach();
         accept_thread_ = std::thread();
     }
     virtual bool Initialize(IOModel _io_model) {
         if ( !_StartWSA(WSAVersion::VERSION_2_2) ) {
             return false;
         }

         io_model_ = _io_model;

         // setting up server config 
         server_ = new TcpServer();
         server_->set_listen_port(9000);
         server_->Start();

         iocp_.StartIocpThread(2);
         accept_thread_ = std::thread(&Network::InternalRun, this);
         return true;
     }

     void InternalRun() {
         TcpClient* client= SetupAcceptableClient();

         // TcpSessionManager.AddTcpClient(client);
         while (IsRunning()) {
             Run();
         }
     }

     bool IsRunning() {
         return is_thread_running_;
     }

     virtual void Run() {
         // watch session count.
         // if network module judges to need new clients, then regist new client session.
        
     }

     virtual void Clean() {
         is_thread_running_ = false;
         ::WaitForSingleObject(get_thread_handle(), INFINITE);
         accept_thread_.detach();
         accept_thread_ = std::thread();

         _EndWSA();
     }

 public:

     virtual const HANDLE get_thread_handle() {
         return accept_thread_.native_handle();
     }


 private:

     TcpClient* SetupAcceptableClient(void) {
         // std::shared_ptr<TcpClient> client = std::shared_ptr<TcpClient>(new TcpClient());
         TcpClient* client = new TcpClient();
         
        // change status
         iocp_.Register((HANDLE)server_->socket(), (DWORD)client);
         
         // bind iocp
         server_->AcceptEx(client);

         // etc..
         return (client);
     }
      

     bool _StartWSA(WSAVersion _version) {
         static WSADATA wsa;
         if (_version == WSAVersion::VERSION_2_2) {
             if (::WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
             {
                 return true;
             }
         }
         return false;
     }
     void _EndWSA(void) {
         ::WSACleanup();
     }

 private:
     IocpBinder iocp_;
     TcpServer* server_;
     IOModel io_model_;
     std::thread accept_thread_;
     bool is_thread_running_;
     int num_init_ccu_;
};


unsigned int __stdcall WorkerThread(LPVOID arg)
{
    HANDLE handle = (HANDLE)arg;
    int retval;
    DWORD transferred;
    TcpClient* client;
    OverlappedIO* overlapped = NULL;

    while (true) {
        retval = ::GetQueuedCompletionStatus(
            handle,
            &transferred,
            (PULONG_PTR)&client,
            (LPOVERLAPPED *)(&overlapped),
            INFINITE);

        // TODO: write down for accepted client.
        if (retval != 1 || client == NULL) {
            // error
        }

        else if (client->accept_overlapped() == overlapped) {
            // on accept
            printf("accpeted!\n");
            client->OnAccept();
        }
        
    }
}
