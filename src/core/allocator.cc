#include "core/allocator.h"
#include <utility>

namespace infini
{
    Allocator::Allocator(Runtime runtime) : runtime(runtime)
    {
        used = 0;
        peak = 0;
        ptr = nullptr;

        // 'alignment' defaults to sizeof(uint64_t), because it is the length of
        // the longest data type currently supported by the DataType field of
        // the tensor
        alignment = sizeof(uint64_t);
    }

    Allocator::~Allocator()
    {
        if (this->ptr != nullptr)
        {
            runtime->dealloc(this->ptr);
        }
    }

    size_t Allocator::alloc(size_t size)
    {
        IT_ASSERT(this->ptr == nullptr);
        // pad the size to the multiple of alignment
        size = this->getAlignedSize(size);

        // =================================== 作业 ===================================
        // TODO: 设计一个算法来分配内存，返回起始地址偏移量
        // =================================== 作业 ===================================

        this->used += size;
        // 从 free_blocks 中找到一个合适的空闲块
        for (auto it = this->free_blocks.begin(); it != this->free_blocks.end(); it++)
        {
            if(it->second >= size)
            {
                size_t addr = it->first;
                size_t space = it->second - size;
                this->free_blocks.erase(it);
                if(space > 0)
                {
                    this->free_blocks[addr + size] = space;
                }
                return addr;
            }
        }
        // 没有找到合适的空闲块，直接分配
        this->peak += size;
        return this->peak - size;
    }

    void Allocator::free(size_t addr, size_t size)
    {
        IT_ASSERT(this->ptr == nullptr);
        size = getAlignedSize(size);

        // =================================== 作业 ===================================
        // TODO: 设计一个算法来回收内存
        // =================================== 作业 ===================================
        // 合并相邻的空闲块

        this->used -= size;
        // 如果释放的内存块正好是最后一个内存块
        if (addr + size == this->peak)
        {
            this->peak -= size;
            return;
        }
        for(auto it = this->free_blocks.begin(); it != this->free_blocks.end(); it++)
        {
            // 如果释放的内存块与某个空闲块相邻，则合并(it,addr)
            if (it->first + it->second == addr)
            {
                it->second += size;
                return;
            }
            // 如果释放的内存块与某个空闲块相邻，则合并(addr,it)
            if (it->first == addr + size)
            {
                this->free_blocks[addr] = size + it->second;
                this->free_blocks.erase(it);
                return;
            }
        }
        this->free_blocks[addr] = size;
    }

    void *Allocator::getPtr()
    {
        if (this->ptr == nullptr)
        {
            this->ptr = runtime->alloc(this->peak);
            printf("Allocator really alloc: %p %lu bytes\n", this->ptr, peak);
        }
        return this->ptr;
    }

    size_t Allocator::getAlignedSize(size_t size)
    {
        return ((size - 1) / this->alignment + 1) * this->alignment;
    }

    void Allocator::info()
    {
        std::cout << "Used memory: " << this->used
                  << ", peak memory: " << this->peak << std::endl;
    }
}
