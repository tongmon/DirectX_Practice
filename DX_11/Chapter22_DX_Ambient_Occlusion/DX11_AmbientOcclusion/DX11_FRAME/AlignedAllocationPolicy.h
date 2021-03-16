#pragma once

// warning C4316 Ã³¸®¿ë
template<size_t T>
class AlignedAllocationPolicy
{
public:
    static void* operator new(size_t size)
    {
        return _aligned_malloc(size, T);
    }

    static void operator delete(void* memory)
    {
        _aligned_free(memory);
    }
};