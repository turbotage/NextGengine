#pragma once

#include "../def.h"
#include "queue.h"

namespace ng {
	namespace threading {

		class ThreadPool {
		private:

			class ThreadWorker {
			private:
				int ID;
				ThreadPool* Pool;
			public:
				ThreadWorker(ThreadPool* pool, const int id)
					: Pool(pool), ID(id) {}

				void operator()();

			};

			bool Shutdown;
			Queue<std::function<void()>> Queue;
			std::vector<std::thread> Threads;
			std::mutex Mutex;
			std::condition_variable Condition;

		public:

			ThreadPool(const int nThreads);

			ThreadPool(const ThreadPool &) = delete;
			ThreadPool(ThreadPool &&) = delete;

			~ThreadPool();

			void init();

			void shutdown();

			template<class F, class... Args>
			auto submit(F&& f, Args&&... args)->std::future<decltype(f(args...))>;

			std::vector<std::thread::id> getThreadIDs();

		};
	}
}

