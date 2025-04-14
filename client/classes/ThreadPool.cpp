#include "../headers.h"

ThreadPool::ThreadPool(size_t numThreads) : m_stop(false), m_activeTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] {
            while (true) {
                function<void()> task;
                unique_lock<mutex> lock(this->m_queueMutex);
                this->m_condition.wait(lock, [this] {
                    return this->m_stop || !this->m_tasks.empty();
                });

                if (this->m_stop && this->m_tasks.empty()) return;

                task = move(this->m_tasks.front());
                this->m_tasks.pop();
                // Execute the task and track the active task count
                m_activeTasks++;

                // Unlock the mutex and allow other threads to work
                lock.unlock();

                try{
                    task();
                }
                catch(const string& e){
                    generalLogger.log("ERROR", "THREAD POOL ERROR!! Error: " + e);
                    // cout << "THREAD POOL ERROR!! Error: " + e + "\n" << flush;
                }
                m_activeTasks--;

                // Notify that a task has completed
                m_waitCondition.notify_one();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (thread &worker : m_workers) worker.join();
}

void ThreadPool::enqueueTask(function<void()> task) {
    {
        unique_lock<mutex> lock(m_queueMutex);
        if (m_stop) throw runtime_error("enqueue on stopped ThreadPool");
        m_tasks.emplace(move(task));
    }
    m_condition.notify_one();
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(m_queueMutex);
    m_waitCondition.wait(lock, [this] { return m_tasks.empty() && m_activeTasks == 0; });
}