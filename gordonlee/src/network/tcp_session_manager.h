#pragma once

#include <list>
#include <memory>

#include "utility/scoped_lock.h"
#include "network/tcp_client.h"

// Singleton

class TcpClient;

class TcpSessionManager {
public:
    virtual ~TcpSessionManager(void) {
    }

    static TcpSessionManager& GetInstance(void) {
        static TcpSessionManager instance;
        return instance;
    }

    bool AddTcpClient(std::shared_ptr<TcpClient> _client) {
        SCOPED_LOCK(TCP_SESSION_MANAGER_LOCK);

        m_clients.remove(_client);
        m_clients.push_back(_client);

        return true;
    }

    void RemoveTcpClient(std::shared_ptr<TcpClient> _client) {
        SCOPED_LOCK(TCP_SESSION_MANAGER_LOCK);

        m_clients.remove(_client);
    }

    std::list<std::shared_ptr<TcpClient>>& GetWholeClients(void) {
        return m_clients;
    }

 public:
     const int get_num_of_clients() const {
         int number = m_clients.size();
         return number;
     }


private:
    TcpSessionManager() {
        m_clients.clear();
    }

private:
    std::list<std::shared_ptr<TcpClient>> m_clients;
};

#define TcpSessionManager TcpSessionManager::GetInstance()