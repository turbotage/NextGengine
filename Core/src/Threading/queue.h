#pragma once

#include "../def.h"

namespace ng {
	namespace threading {

		template <typename T>
		class Queue
		{
		private:
			std::queue<T> Queue;
			std::mutex Mutex;
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
			std::lock_guard<std::mutex> lock(Mutex);
			return Queue.empty();
		}

		template<typename T>
		int Queue<T>::size()
		{
			std::lock_guard<std::mutex> lock(Mutex);
			return Queue.size();
		}

		template<typename T>
		void Queue<T>::enqueue(T& t)
		{
			std::lock_guard<std::mutex> lock(Mutex);
			Queue.push(t);
		}

		template<typename T>
		bool Queue<T>::dequeue(T& t)
		{
			std::lock_guard<std::mutex> lock(Mutex);

			if (Queue.empty()) {
				return false;
			}
			t = std::move(Queue.front());

			Queue.pop();
			return true;
		}

	}
}


