#include "Scheduler.h"
#include <chrono>
#include <thread>

int main() {
    // 创建调度器，使用4个工作线程
    Scheduler scheduler(4);

    // Task 1: 根任务，无依赖，优先级1，执行500ms
    scheduler.addTask(1, 1, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }, {});  // 空依赖列表，入度为0

    // Task 2: 依赖Task 1，优先级2，执行300ms
    scheduler.addTask(2, 2, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }, {1});  // 依赖Task 1

    // Task 3: 依赖Task 1，优先级3（最高），执行200ms
    scheduler.addTask(3, 3, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }, {1});  // 依赖Task 1

    // Task 4: 依赖Task 2和3，优先级1，执行100ms
    scheduler.addTask(4, 1, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }, {2, 3});  // 依赖Task 2和Task 3（入度为2）

    // 启动调度器，开始执行所有任务
    scheduler.run();
    
    return 0;
}
