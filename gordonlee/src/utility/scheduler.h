#pragma once

#include <thread>
#include <map>

#include "core_header\base_header.h"
#include "utility\job_processor.h"
#include "utility\scoped_lock.h"

class SchedulerElement {
public:
    SchedulerElement(std::shared_ptr<IJob> job_, JobProcessor* processor_)
        : _job(job_), _processor(processor_) {
    }

    ~SchedulerElement() {
        _job.reset();
    }

    std::shared_ptr<IJob> job(void) {
        return _job;
    }

    JobProcessor* processor(void) {
        return _processor;
    }

private:
    std::shared_ptr<IJob> _job;
    JobProcessor* _processor;
};

class Scheduler {
public:
    virtual ~Scheduler() {
    }

    static Scheduler& GetInstance() {
        static Scheduler instance;
        return instance;
    }

    void WorkerProccess() {

        std::chrono::time_point<std::chrono::system_clock> now;
        while (true) {
            now = std::chrono::system_clock::now();
            AutoLock lock(&cs_);

            if (!job_map_.empty()) {
                auto& iterator = job_map_.begin();
                if (now >= iterator->first) {
                    auto& element = iterator->second;
                    element->processor()->Enqueue(element->job());
                    job_map_.erase(iterator);
                }
            }
            else {
            }
        }
    }

    void Schedule(JobProcessor* processor, IJob job) {
        Schedule(processor, job, std::chrono::milliseconds(0));
    }

    void Schedule(JobProcessor* processor, IJob job_, __int64 millis_) {
        std::chrono::milliseconds cast_timespan(millis_);
        Schedule(processor, job_, cast_timespan);
    }

    void Schedule(JobProcessor* processor, IJob job, std::chrono::milliseconds timespan) {

        std::chrono::time_point<std::chrono::system_clock> key
            = std::chrono::system_clock::now() + timespan;

        SchedulerElement* ptr = new SchedulerElement(
            std::shared_ptr<IJob>(new IJob(job)),
            processor);

        std::pair<
            std::chrono::time_point<std::chrono::system_clock>,
            std::shared_ptr<SchedulerElement >> mapPair(key, std::shared_ptr<SchedulerElement>(ptr));

        {
            AutoLock lock(&cs_);
            job_map_.insert(mapPair);
        }
    }

private:
    Scheduler() {
        job_map_.clear();
        thread_ = std::thread(&Scheduler::WorkerProccess, this);
    }

private:
    CriticalSectionLock cs_;
    std::map<std::chrono::time_point<std::chrono::system_clock>, std::shared_ptr<SchedulerElement>> job_map_;
    std::thread thread_;
};

