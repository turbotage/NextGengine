#pragma once

#include "thread_safe_queue.h"

#include <memory>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <future>

namespace ng {

	class ThreadPool {
	public:
		/**
		 * Constructor.
		 */
		ThreadPool(void)
			:ThreadPool{ std::max(std::thread::hardware_concurrency(), 2u) - 1u }
		{
			/*
			 * Always create at least one thread.  If hardware_concurrency() returns 0,
			 * subtracting one would turn it to UINT_MAX, so get the maximum of
			 * hardware_concurrency() and 2 before subtracting 1.
			 */
		}

		/** * Constructor. */
		explicit ThreadPool(const std::uint32_t numThreads);

		/** * Non-copyable. */
		ThreadPool(const ThreadPool& rhs) = delete;

		/** * Non-assignable. */
		ThreadPool& operator=(const ThreadPool& rhs) = delete;

		/** * Destructor. */
		~ThreadPool(void) { destroy(); }

		template <typename Func, typename... Args>
		auto submit(Func&& func, Args&&... args);

	public:

		/**
		 * A wrapper around a std::future that adds the behavior of futures returned from std::async.
		 * Specifically, this object will block and wait for execution to finish before going out of scope.
		 */
		template <typename T>
		class TaskFuture
		{
		public:
			TaskFuture(std::future<T>&& future)
				:m_Future{ std::move(future) }
			{
			}

			TaskFuture(const TaskFuture& rhs) = delete;
			TaskFuture& operator=(const TaskFuture& rhs) = delete;
			TaskFuture(TaskFuture&& other) = default;
			TaskFuture& operator=(TaskFuture&& other) = default;
			~TaskFuture(void)
			{
				if (m_Future.valid())
				{
					m_Future.get();
				}
			}

			auto get(void)
			{
				return m_Future.get();
			}


		private:
			std::future<T> m_Future;
		};

	private:
		/** * Constantly running function each thread uses to acquire work items from the queue. */
		void worker();

		/** * Invalidates the queue and joins all running threads. */
		void destroy();

	private:
		class IThreadTask
		{
		public:
			IThreadTask(void) = default;
			virtual ~IThreadTask(void) = default;
			IThreadTask(const IThreadTask& rhs) = delete;
			IThreadTask& operator=(const IThreadTask& rhs) = delete;
			IThreadTask(IThreadTask&& other) = default;
			IThreadTask& operator=(IThreadTask&& other) = default;

			/**
			 * Run the task.
			 */
			virtual void execute() = 0;
		};

		template <typename Func>
		class ThreadTask : public IThreadTask
		{
		public:
			ThreadTask(Func&& func)
				: m_Func{ std::move(func) }
			{
			}

			~ThreadTask(void) override = default;
			ThreadTask(const ThreadTask& rhs) = delete;
			ThreadTask& operator=(const ThreadTask& rhs) = delete;
			ThreadTask(ThreadTask&& other) = default;
			ThreadTask& operator=(ThreadTask&& other) = default;

			/**
			 * Run the task.
			 */
			void execute() override
			{
				m_Func();
			}

		private:
			Func m_Func;
		};

	private:
		std::atomic_bool m_Done;
		ThreadSafeQueue<std::unique_ptr<IThreadTask>> m_WorkQueue;
		std::vector<std::thread> m_Threads;

	};

	template<typename Func, typename ...Args>
	inline auto ThreadPool::submit(Func&& func, Args&& ...args)
	{
		auto boundTask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
		using ResultType = std::result_of_t<decltype(boundTask)()>;
		using PackagedTask = std::packaged_task<ResultType()>;
		using TaskType = ThreadTask<PackagedTask>;

		PackagedTask task{ std::move(boundTask) };
		TaskFuture<ResultType> result{ task.get_future() };
		m_workQueue.push(std::make_unique<TaskType>(std::move(task)));
		return result;
	}

#ifdef NG_USE_DEFAULT_THREAD_POOL

	namespace threadpool {
		/**
		 * Get the default thread pool for the application.
		 * This pool is created with std::thread::hardware_concurrency() - 1 threads.
		 */
		inline ThreadPool& getThreadPool() {
			static ThreadPool defaultPool;
			return defaultPool;
		}

		/**
		 * Submit a job to the default thread pool.
		 */
		template <typename Func, typename... Args>
		inline auto submitJob(Func&& func, Args&&... args)
		{
			return getThreadPool().submit(std::forward<Func>(func), std::forward<Args>(args)...);
		}
	}

#endif

}
