#pragma once
#include <functional>
#include <vector>
#include <atomic>

// Task：表示调度系统中的一个可执行任务
// 拓扑排序 + 优先级调度
struct Task {
    int id;
    int priority;
    std::function<void()> func;

    // 剩余入度：有多少前置任务未完成
    std::atomic<int> remainingDeps{0};

    // 后继任务列表：当前任务完成后需通知哪些任务（出度）
    std::vector<int> nextTasks;

    Task(int id_, int priority_, std::function<void()> f)
        : id(id_), priority(priority_), func(std::move(f)) {}
};