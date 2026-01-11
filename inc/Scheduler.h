#pragma once
#include "ThreadPool.h"
#include "Task.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

// 任务调度器
class Scheduler {
public:
    // 构造函数：创建调度器并初始化线程池
    explicit Scheduler(size_t numThreads);

    void addTask(int id,
                 int priority,
                 std::function<void()> func,
                 const std::vector<int>& dependencies);
    
    // 启动调度器，开始执行任务
    void run();

private:
    struct TaskCompare {
        bool operator()(const std::shared_ptr<Task>& a,
                        const std::shared_ptr<Task>& b) {
            return a->priority < b->priority;
        }
    };

    // 调度单个任务到线程池执行
    void scheduleTask(const std::shared_ptr<Task>& task);

    ThreadPool pool_;
    // 任务映射表（ID -> Task）
    std::unordered_map<int, std::shared_ptr<Task>> tasks_;

    // 就绪队列：入度为0且按优先级排序的任务队列
    std::priority_queue<
        std::shared_ptr<Task>,
        std::vector<std::shared_ptr<Task>>,
        TaskCompare    
    > readyQueue_;

    std::mutex mutex_;
    // 等待所有任务完成的条件变量
    std::condition_variable allDoneCV_;
    std::atomic<int> completedTasks_{0};
    std::atomic<int> totalTasks_{0};
};

