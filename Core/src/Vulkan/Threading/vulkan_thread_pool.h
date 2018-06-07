#pragma once

#include "../../def.h"

namespace ng {
	namespace vulkan {
		class VulkanDevice;
	}
}

namespace ng {
	namespace vulkan {

		/* creates one thread pool for a vulkan_device and a thread_pool with one thread for the memory transfers the device should handle */
		class VulkanThreadPool
		{
		private:


			class VulkanThread {
			private:

				void run() {
					std::function<void()> func;
					while (!stop) {
						{
							std::unique_lock<std::mutex> lock(mutex);
							if (workQueue.empty()) {
								condition.wait(lock);
								if (stop) {
									return;
								}
							}
							func = std::move(workQueue.front());
							workQueue.pop();
						}

						func();
						--nTasks;
					}
				}

			public:
				std::thread thread;
				std::queue<std::function<void()>> workQueue;
				
				uint8 id;
				
				VkCommandPool commandPool;
				std::list<VkCommandBuffer> commandBuffers;

				std::atomic<bool> stop = false;

				std::mutex mutex;
				std::condition_variable condition;

				std::atomic<int> nTasks;
				
				VulkanThread(uint8 id) 
					: thread(this->run)
				{
					this->id = id;
				}

				~VulkanThread() {

				}

				void stop() {
					stop = true;
					condition.notify_all();
				}

			};
			
			std::vector<VulkanThread> m_WorkPool;
			
			VulkanDevice* m_VulkanDevice;

		public:

			VulkanThreadPool(const int nWorkThreads, ng::vulkan::VulkanDevice* vulkanDevice, bool memTransferSamePool = false);

			VulkanThreadPool(const VulkanThreadPool &) = delete;
			VulkanThreadPool(VulkanThreadPool &&) = delete;

			~VulkanThreadPool();

			void shutdown();

			VkCommandBuffer getSecondaryCommandBuffer(uint16 i);

			template<class F, class... Args>
			auto submitWork(uint16 threadID, F&& f, Args&&... args)->std::future<decltype(f(args...))>;

		};

		template<class F, class ...Args>
		inline auto VulkanThreadPool::submitWork(uint16 threadID, F && f, Args && ...args) -> std::future<decltype(f(args ...))>
		{
			using return_type = typename std::result_of<F(Args...)>::type;

			if (!(threadID > 0 && threadID <= WorkPool.size())) {
				throw std::runtime_error("called submitWork on non-exsisting work thread");
			}

			auto task = std::make_shared<std::packaged_task<return_type()> >(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...)
				);

			std::future<return_type> res = task->get_future();
			{
				if (WorkPool[threadID].stop) {
					throw std::runtime_error("tried to submit work to stopped vulkan work-thread!");
				}

				std::lock_guard<std::mutex> lock(WorkPool[threadID].mutex);

				WorkPool[threadID].workQueue.push([task]() { (*task)(); });
				++WorkPool[threadID].nTasks;
			}

			WorkPool[threadID].condition.notify_one();
			return res;
		}

	}
}


