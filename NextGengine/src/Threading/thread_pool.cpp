#include "thread_pool.h"



ng::ThreadPool::ThreadPool(const std::uint32_t numThreads)
    : m_Done{ false }, m_WorkQueue{}, m_Threads{}
{
    try
    {
        for (std::uint32_t i = 0u; i < numThreads; ++i)
        {
            m_threads.emplace_back(&ThreadPool::worker, this);
        }
    }
    catch (...)
    {
        destroy();
        throw;
    }
}

void ng::ThreadPool::worker()
{
    while (!m_Done)
    {
        std::unique_ptr<IThreadTask> pTask{ nullptr };
        if (m_workQueue.waitPop(pTask))
        {
            pTask->execute();
        }
    }
}

void ng::ThreadPool::destroy()
{
    m_Done = true;
    m_WorkQueue.invalidate();
    for (auto& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
