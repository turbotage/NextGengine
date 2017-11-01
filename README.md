# NextGengine

#Using readme as a engine flow reminder and planner

    Query available device local memory
    Query available host local, host visible memory
    --> Find biggest common allocation size : CAS (common allocation size)
        --> Allocate CAS amount of host visible memory : HVM (host visible memory)
        --> Allocate CAS amount of device local memory : DLM (device local memory)
            --> Divide DLM into one vertex buffer one index buffer and one uniform buffer of appropriate 
                sizes : VBS (vertex buffer size), IBS (index buffer size), UBS (uniform buffer size)
        --> Init VulkanMemoryAllocator
            --> For VBS init one FreeListAllocator
            --> For IBS init one FreeList Allocator
            --> for UBS init one FixedBlockAllocator

    Query available device local, host visible memory
    Query available host local, host visible memory
    --> Find biggest common allocation size : CAS (common allocation size)
        --> Allocate CAS amount of host local, host visible memory : HVM (host visible memory)
        --> Allocate CAS amount of device local, host visible memory : DLM (device local memory)
            --> Divide DLM into one vertex buffer one index buffer and one uniform buffer of appropriate 
                sizes : VBS (vertex buffer size), IBS (index buffer size), UBS (uniform buffer size)
        --> Init VulkanMemoryAllocator
            --> For VBS init one FreeListAllocator
            --> For IBS init one FreeList Allocator
            --> for UBS init one FixedBlockAllocator
            