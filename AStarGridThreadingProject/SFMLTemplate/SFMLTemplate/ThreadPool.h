// Author: Robin Meyler
// Date: 2021
// Title: Thread Pool Implmentation of A* Ambush

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "Globals.h"
class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();
	void addTask(std::function<void()>);
	void waitForTaskInifiteLoop();
private:
	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_tasks;
	std::mutex queueMutex;
	std::condition_variable threadCondition;
};
#endif // !THREAD_POOL_H

