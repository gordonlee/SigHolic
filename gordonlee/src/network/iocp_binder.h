#pragma once
#include <process.h>

#include "core_header\base_header.h"

extern unsigned int __stdcall WorkerThread(LPVOID arg);

class IocpBinder {
 public:
	 IocpBinder(void) : iocp_handle_(0) {}
	 virtual ~IocpBinder(void) {}

	 int StartIocpThread(DWORD _num_of_thread) {

		 if ( _CreateIocpHandle() < 0) {
			 return -1;
		 }

		 HANDLE thread_handle;
		 unsigned int thread_id;
		 for (DWORD i = 0; i < _num_of_thread; ++i) {
			 thread_handle = (HANDLE)_beginthreadex(0, 0, WorkerThread, iocp_handle_, 0, &thread_id);
			 if (thread_handle == NULL) {
				 return -2;
			 }
			 ::CloseHandle(thread_handle);
		 }

		 return 0;
	 }

	 int Register(HANDLE _socket, DWORD _completion_key) {
		 HANDLE result = ::CreateIoCompletionPort(_socket, iocp_handle_, _completion_key, 0);
		 if (result == NULL) {
			 return -1;
		 }
		 return 0;
	 }

 public:
	 HANDLE iocp_handle(void) {
		 return iocp_handle_;
	 }

 private:
	 int _CreateIocpHandle(void) {
		 HANDLE iocp_handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		 if (iocp_handle == NULL) {
			 return -1;
		 }

		 iocp_handle_ = iocp_handle;
		 return 0;
	 }

 private:
	 HANDLE iocp_handle_;
};