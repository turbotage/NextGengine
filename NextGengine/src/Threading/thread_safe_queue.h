#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace ng {
	
	template<typename T>
	class ThreadSafeQueue {
	public:

		~ThreadSafeQueue();

		bool tryPop(T& out);

		bool waitPop(T& out);

		void push(T value);

		bool empty() const;

		void clear();

		void invalidate();

		bool isValid() const;

	private:
		std::atomic_bool m_Valid{ true };
		mutable std::mutex m_Mutex;
		std::queue<T> m_Queue;
		std::condition_variable m_Condition;
	};

	template<typename T>
	inline ThreadSafeQueue<T>::~ThreadSafeQueue()
	{
		invalidate();
	}

	template<typename T>
	inline bool ThreadSafeQueue<T>::tryPop(T& out)
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		if (m_Queue.empty() || !m_Valid) {
			return false;
		}
		out = std::move(m_Queue.front());
		m_Queue.pop();
		return true;
	}

	template<typename T>
	inline bool ThreadSafeQueue<T>::waitPop(T& out)
	{
		std::unique_lock<std::mutex> lock{ m_Mutex };
		m_Condition.wait(lock, [this]() {
			return !m_Queue.empty() || !m_Valid;
		});

		if (!m_Valid) {
			return false;
		}
		out = std::move(m_Queue.front());
		m_Queue.pop();
		return true;
	}

	template<typename T>
	inline void ThreadSafeQueue<T>::push(T value)
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		m_Queue.push(std::move(value));
		m_Condition.notify_one();
	}

	template<typename T>
	inline bool ThreadSafeQueue<T>::empty() const
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		return m_Queue.empty();
	}

	template<typename T>
	inline void ThreadSafeQueue<T>::clear()
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		while (!m_Queue.empty()) {
			m_Queue.pop();
		}
		m_Condition.notify_all();
	}

	template<typename T>
	inline void ThreadSafeQueue<T>::invalidate()
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		m_Valid = false;
		m_Condition.notify_all();
	}

	template<typename T>
	inline bool ThreadSafeQueue<T>::isValid() const
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		return m_Valid;
	}

}