#pragma once
#include <vector>
#include <thread>
#include <queue> 
#include <mutex> 
#include <condition_variable> 
#include <functional> 
#include <atomic>

class ThreadPool {
public:
    // 强制程序员必须明确写出“我要创建一个线程池”，禁止隐式类型转换。
    explicit ThreadPool(size_t numThreads);
    
    ~ThreadPool();

    // 提交任务到线程池（使用lock_guard保证任务队列的线程安全访问）。
    void submit(std::function<void()> task);

    // 关闭线程池，等待所有工作线程退出。
    void shutdown();

private:
    void workerLoop();

    // std::thread:工作线程数组（C++并发的载体）。
    std::vector<std::thread> workers_;
    // 任务队列（FIFO）
    std::queue<std::function<void()>> tasks_;

    // 保护任务队列的互斥锁。
    std::mutex mutex_;
    // 条件变量，用于线程等待和唤醒。
    std::condition_variable cv_;
    // 停止标志（原子操作）
    std::atomic<bool> stop_{false};
};
