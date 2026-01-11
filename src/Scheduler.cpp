#include "Scheduler.h"
#include "Task.h"
#include "ThreadPool.h"
#include <iostream>
#include <memory>
#include <mutex>

Scheduler::Scheduler(size_t numThreads)
    : pool_(numThreads) {}

// 添加任务并建立依赖关系
void Scheduler::addTask(int id,
                       int priority,
                       std::function<void()> func,
                       const std::vector<int>& dependencies) {
    auto task = std::make_shared<Task>(id, priority, func);
    
    // 设置入度
    task->remainingDeps = dependencies.size();

    // 保证线程安全地更新任务映射表
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_[id] = task;

    // 建立依赖关系
    for (int dep : dependencies) {
        tasks_[dep]->nextTasks.push_back(id);
    }
    ++totalTasks_;
}

// 启动调动器，执行所有任务
void Scheduler::run() {
    // 初始化就绪队列
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, task] : tasks_) {
            if (task->remainingDeps == 0) {
                readyQueue_.push(task);
            }
        }
    }

    // 循环调度任务
    while (true) {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (readyQueue_.empty()) break;
            task = readyQueue_.top();
            readyQueue_.pop();
        }

        scheduleTask(task);
    }

    // 等待所有任务完成
    std::unique_lock<std::mutex> lock(mutex_);
    allDoneCV_.wait(lock, [this] {
        return completedTasks_ >= totalTasks_;
    });
}

// 调度单个任务到线程池执行
void Scheduler::scheduleTask(const std::shared_ptr<Task>& task) {
    pool_.submit([this, task]() {
        std::cout << "[Task " << task->id << "] start\n";
        task->func();
        std::cout << "[Task " << task->id << "] end\n";
        
        // 任务完成，更新依赖
        std::lock_guard<std::mutex> lock(mutex_);
        for (int nextId : task->nextTasks) {
            auto& nextTask = tasks_[nextId];

            if(--nextTask->remainingDeps == 0) {
                // 入度归0，任务就绪
                readyQueue_.push(nextTask);
            }
        }

        while (!readyQueue_.empty()) {
            auto taskToSchedule = readyQueue_.top();
            readyQueue_.pop();
            scheduleTask(taskToSchedule);
        }

        ++completedTasks_;
        // 所有任务完成，通知等待线程
        if (completedTasks_ >= totalTasks_)
            allDoneCV_.notify_one();
    });
}