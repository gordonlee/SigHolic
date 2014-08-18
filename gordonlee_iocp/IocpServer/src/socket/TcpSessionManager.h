#pragma once

#include <list>
#include <memory>

#include "utility/AutoLock.h"
#include "socket/TcpClient.h"

// Singleton
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

 private:
	 TcpSessionManager() {
		 m_clients.clear();
	 }

 private:
	 std::list<std::shared_ptr<TcpClient>> m_clients;
};

#define TcpSessionManager TcpSessionManager::GetInstance()