#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace ng {

	class Thread {
	public:

		Thread();
		~Thread();

		void addJob(std::function<void()> function);

		void wait();

	private:

		void queueLoop();

	private:
		bool m_Destroying = false;
		std::thread m_Worker;
		std::queue<std::function<void()>> m_JobQueue;
		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;

	};

	class ThreadPool {
	public:
		std::vector<std::unique_ptr<Thread>> threads;

		void setThreadCount(uint32 count);

		void wait();
	};

}


