#include "ThreadPool.h"
#include <mutex>

// 构造函数：创建并启动工作线程
ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] {
            this->workerLoop(); 
        });
    }
}

// 析构函数：RAII自动资源管理
ThreadPool::~ThreadPool() {
    shutdown();
}

// 提交任务到线程池
void ThreadPool::submit(std::function<void()> task) {
    // 利用RAII机制，当lock对象被创建，自动锁住mutex_
    // {} 控制锁的作用域，确保被唤醒的线程不会阻塞在锁上（提前解锁），立刻工作
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    // 唤醒一个等待的工作线程。
    cv_.notify_one();
}

void ThreadPool::shutdown() {
    stop_ = true;
    // 唤醒所有线程，让它们检查stop标志
    cv_.notify_all(); 
    
    // 等待所有线程退出（join）
    for (auto& t : workers_) {
        if (t.joinable()) {
            t.join(); // 阻塞进程直到线程结束
        }
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        // 定义一个空的函数容器，用来暂存从队列取出的任务
        std::function<void()> task;
        {
            // 进入任务队列前先加锁，防止多个线程取同一个任务
            std::unique_lock<std::mutex> lock(mutex_);

            // 等待直到：有任务可执行 或 收到停止信号
            cv_.wait(lock, [&] {
                return stop_ || !tasks_.empty();
            });

            // 确保处理完所有提交的任务
            if (stop_ && tasks_.empty()) {
                return;
            }

            // 取出任务（移动语义避免拷贝）
            task = std::move(tasks_.front());
            tasks_.pop();
        } // 离开作用域，自动释放锁

        // 锁外执行任务，避免阻塞其它线程提交或获取任务
        task();
    }
}
