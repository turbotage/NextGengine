#pragma once

#include "../def.h"

namespace ng {
	namespace threading {

		template <typename T>
		class Queue
		{
		private:
			std::queue<T> m_Queue;
			std::mutex m_Mutex;
		public:
			Queue();
			~Queue();

			bool empty();

			int size();

			void enqueue(T& t);

			bool dequeue(T& t);

		};


		template<typename T>
		Queue<T>::Queue()
		{
		}

		template<typename T>
		Queue<T>::~Queue()
		{
		}

		template<typename T>
		bool Queue<T>::empty()
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			return m_Queue.empty();
		}

		template<typename T>
		int Queue<T>::size()
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			return m_Queue.size();
		}

		template<typename T>
		void Queue<T>::enqueue(T& t)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push(t);
		}

		template<typename T>
		bool Queue<T>::dequeue(T& t)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			if (m_Queue.empty()) {
				return false;
			}
			t = std::move(m_Queue.front());

			m_Queue.pop();
			return true;
		}

	}
}


