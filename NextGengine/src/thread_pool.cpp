#include "thread_pool.h"

// <=================== THREAD =====================>
//public
ng::Thread::Thread()
{
	m_Worker = std::thread(&Thread::queueLoop, this);
}

ng::Thread::~Thread()
{
	if (m_Worker.joinable()) {
		wait();
		m_QueueMutex.lock();
		m_Destroying = true;
		m_Condition.notify_one();
		m_QueueMutex.unlock();
		m_Worker.join();
	}
}

void ng::Thread::addJob(std::function<void()> function)
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	m_JobQueue.push(std::move(function));
	m_Condition.wait(lock, [this]() { return m_JobQueue.empty(); });
}

void ng::Thread::wait()
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_Condition.wait(lock, [this]() { return m_JobQueue.empty(); });
}

//private
void ng::Thread::queueLoop()
{
	while (true) {
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_Condition.wait(lock, [this]() { return !m_JobQueue.empty() || m_Destroying; });
			if (m_Destroying) {
				break;
			}
			job = m_JobQueue.front();
		}

		job();

		{
			std::lock_guard<std::mutex> lock(m_QueueMutex);
			m_JobQueue.pop();
			m_Condition.notify_one();
		}
	}
}


//<================= THREAD POOL ======================>
//public
void ng::ThreadPool::setThreadCount(uint32 count)
{
	threads.clear();
	for (auto i = 0; i < count; ++i) {
		threads.push_back(std::make_unique<Thread>());
	}
}

void ng::ThreadPool::wait()
{
	for (auto& thread : threads)
	{
		thread->wait();
	}
}