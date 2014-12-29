#pragma once
#include <functional>
#include <thread>
#include <concurrent_queue.h>

#include "core_header\base_header.h"

class IJob {
public:
    IJob(){  }
    IJob(std::function<void()> func) : _func(func) {
    }

    void Do() {
        if (_func)
        {
            _func();
        }
    }

private:
    std::function<void()> _func;
};

class JobProcessor {
public:

    JobProcessor() {
        event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        thread_ = std::thread(&JobProcessor::WorkerProccess, this);
    }

    ~JobProcessor() {
        thread_.detach();
        thread_ = std::thread();
        ::CloseHandle(event_);
    }

    void WorkerProccess() {
        std::shared_ptr<IJob> job;
        int spin_count = 0;
        while (true) {
            job = Dequeue();
            if (job != NULL && job.get() != NULL) {
                job->Do();
                spin_count = 0;
            }
            else {
                ++spin_count;
            }

            if (spin_count > spin_count) {
                ::WaitForSingleObject(event_, INFINITE);
            }
        }
    }

    bool Enqueue(std::shared_ptr<IJob> job_) {
        queue_.push(job_);

        ::SetEvent(event_);

        return true;
    }

    std::shared_ptr<IJob> Dequeue(void) {
        std::shared_ptr<IJob> job;
        queue_.try_pop(job);

        return job;
    }

private:
    concurrency::concurrent_queue<std::shared_ptr<IJob>> queue_;
    std::thread thread_;
    HANDLE event_;
    static const int spin_count = 100;
};
