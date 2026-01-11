# Task Scheduler —— 基于 C++ 的多线程任务调度框架

## 项目简介

本项目使用 **C++17** 实现了一个轻量级的多线程任务调度框架，支持：

* 线程池并发执行
* 任务依赖关系（DAG）
* 基于优先级的任务调度
* 线程安全的任务管理与同步

项目重点关注 **并发控制、资源管理与工程结构设计**，适合作为系统/基础设施方向的工程实践示例。


## 项目结构

```
task_scheduler/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── ThreadPool.h      # 线程池抽象
│   ├── Task.h            # 任务数据结构
│   └── Scheduler.h       # 调度器核心逻辑
├── src/
│   ├── ThreadPool.cpp
│   ├── Scheduler.cpp
│   └── main.cpp          # 示例入口
```

## 核心模块说明

### 1. ThreadPool（线程池）

* 固定数量工作线程
* 使用 `std::queue` 存储待执行任务
* 基于 `std::condition_variable` 实现阻塞等待，避免 busy waiting
* 支持安全 shutdown，保证线程可控退出

**关键设计点：**

* 使用 RAII 管理线程生命周期
* 条件变量唤醒逻辑：
  `stop || !tasks.empty()`

### 2. Task 模型

每个任务包含：

* 唯一 Task ID
* 优先级（用于调度顺序）
* 可执行函数（`std::function<void()>`）
* 剩余依赖数（入度）
* 后继任务列表

**依赖关系建模方式：**

* 整体任务依赖构成一个 **DAG**
* 使用“入度归零”作为任务可执行条件

### 3. Scheduler（调度器）

Scheduler 负责：

* 管理任务生命周期
* 维护 ready queue（优先级队列）
* 在任务完成后触发后继任务调度

**调度流程：**

1. 初始化阶段将入度为 0 的任务加入 ready queue
2. 从 ready queue 中按优先级取出任务
3. 提交给线程池执行
4. 任务完成后更新依赖计数
5. 新解锁的任务进入 ready queue

## 并发与同步策略

* **互斥量（mutex）**：

  * 保护任务表与 ready queue
* **条件变量（condition_variable）**：

  * 线程池任务调度
* **原子变量（atomic）**：

  * 任务剩余依赖数（in-degree）

该设计在保证正确性的前提下，尽量减少锁粒度，避免不必要的阻塞。

## 构建与运行

### 构建

```bash
mkdir build
cd build
cmake ..
make
```

### 运行

```bash
./task_scheduler
```

## 技术栈

* C++17
* STL（thread / mutex / condition_variable / atomic）
* CMake
* Linux / macOS