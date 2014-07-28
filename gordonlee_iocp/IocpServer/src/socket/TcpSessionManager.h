#pragma once

#include <list>

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

	 bool AddTcpClient(TcpClient* _client) {
		 SCOPED_LOCK(TCP_SESSION_MANAGER_LOCK);
		 
		 m_clients.remove(_client);
		 m_clients.push_back(_client);

		 return true;
	 }

	 void RemoveTcpClient(TcpClient* _client) {
		 SCOPED_LOCK(TCP_SESSION_MANAGER_LOCK);

		 m_clients.remove(_client);
	 }

	 std::list<TcpClient*>& GetWholeClients(void) {
		 return m_clients;
	 }

 private:
	 TcpSessionManager() {
		 m_clients.clear();
	 }

 private:
	 std::list<TcpClient*> m_clients;
};

#define TcpSessionManager TcpSessionManager::GetInstance()