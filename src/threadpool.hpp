#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>


#ifndef RFSS_THREADPOOL_HPP
#define RFSS_THREADPOOL_HPP

namespace index_stream {
    class ThreadPool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        std::condition_variable all_tasks_done_condition;
        std::atomic<int> active_tasks;
        bool pause;
        bool stop;

    public:
        ThreadPool(size_t num_threads);
        ~ThreadPool();

        void pause_task_queue();
        void resume_task_queue();
        bool await_pending_tasks();

        template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args) -> void {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (stop)   
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            }
            condition.notify_one();
        }
    };
}

#endif