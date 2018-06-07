#include "thread_pool.h"


void ng::threading::ThreadPool::ThreadWorker::operator()()
{
	std::function<void()> func;
	bool dequeued;
	while (!Pool->Shutdown) {
		std::unique_lock<std::mutex> lock(Pool->Mutex);
		if (Pool->Queue.empty()) {
			Pool->Condition.wait(lock);
		}
		dequeued = Pool->Queue.dequeue(func);
	}
	if (dequeued) {
		func();
	}
}

ng::threading::ThreadPool::ThreadPool(const int nThreads)
	: Threads(std::vector<std::thread>(nThreads)), Shutdown(false)
{
	
}

ng::threading::ThreadPool::~ThreadPool()
{
	shutdown();
}

void ng::threading::ThreadPool::init()
{
	for (int i = 0; i < Threads.size(); ++i) {
		Threads[i] = std::thread(ThreadWorker(this, i));
	}
}

void ng::threading::ThreadPool::shutdown()
{
	Shutdown = true;
	Condition.notify_all();

	for (int i = 0; i < Threads.size(); ++i) {
		if (Threads[i].joinable()) {
			Threads[i].join();
		}
	}

}

std::vector<std::thread::id> ng::threading::ThreadPool::getThreadIDs()
{
	std::vector<std::thread::id> ret;
	for (auto& thread : Threads) {
		ret.push_back(thread.get_id());
	}
	return ret;
}


template<class F, class ...Args>
auto ng::threading::ThreadPool::submit(F && f, Args && ...args) -> std::future<decltype(f(args ...))>
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{

		if (Stop.load()) {
			throw std::runtime_error("enqueue on stopped ThreadPool");
		}

		Queue.enqueue([task]() { (*task)(); });

	}

	Condition.notify_one();
	return res;
}