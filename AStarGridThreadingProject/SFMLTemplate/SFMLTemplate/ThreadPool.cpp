#include "ThreadPool.h"

ThreadPool::ThreadPool() {
	int cores = std::thread::hardware_concurrency() - 1;
	for (int i = 0; i < cores; i++)
	{
		std::thread listner(&ThreadPool::waitForTaskInifiteLoop, this);
		m_threads.push_back(move(listner));
	}
}

ThreadPool::~ThreadPool() {}


void ThreadPool::addTask(std::function<void()> task)
{
	std::unique_lock<std::mutex> lock(queueMutex);	// Lock to avoid issues
	m_tasks.push(task);								// Push the new task into the queue
	threadCondition.notify_one();					// Notify 1 thread waiting that a task is available
}

void ThreadPool::waitForTaskInifiteLoop()
{
	std::function<void()> task;
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);	// Lock
			threadCondition.wait(lock, [&] {return !m_tasks.empty(); });						// Wait for Task to be added
			task = std::move(m_tasks.front());							// Get the task 
			m_tasks.pop();
		}	// Remove task from the task queue
		task();											// execute the task
	}
}
