/******************************************************************************/
/*
* @file   memoryallocator.h
* @author Aditya Harsh
* @brief  Fast and safe memory-allocator. Fixed-size.
*/
/******************************************************************************/

#pragma once

#include <cstring>   /* std::memset        */
#include <stdexcept> /* std::runtime_error */
#include <memory>    /* std::align         */

// macros to make integration easier if making a static class allocator
#define CREATE_CLASS_NEW(_alloc_name)                                               \
void* operator new(std::size_t)                                                     \
{                                                                                   \
    void* mem = _alloc_name.AlignCheck(_alloc_name.MemoryAllocator::Allocate());    \
    if (!mem) throw std::runtime_error("Allocation failed.");                       \
    return mem;                                                                     \
}                                                                                   \

#define CREATE_CLASS_DELETE(_alloc_name) void operator delete(void* ptr) noexcept {_alloc_name.MemoryAllocator::Free(ptr);}
#define GEN_CLASS_NEW_DEL(_alloc_name) CREATE_CLASS_NEW(_alloc_name) CREATE_CLASS_DELETE(_alloc_name)

namespace ATL
{
    template <size_t block_size, size_t blocks>
    class MemoryAllocator
    {
        // safety checking
        static_assert(block_size >= 1, "Block size must be at least 1 byte.");
        static_assert(blocks >= 1, "At least 1 block must be allocated.");

        // internal memory type
        using uchar = unsigned char;

        // byte patterns to mark memory blocks
        enum Pattern : uchar
        {
            UNALLOCATED = 0xAA,
            ALLOCATED = 0xBB
        };

        // represents a list object
        struct List
        {
            List* next = nullptr;
        };

        // internal memory block
        uchar* data_;
        // page of available blocks
        List* free_list_;

        // meta data
        static constexpr size_t pad_bytes = 2;
        static constexpr size_t vp_size = sizeof(void*);
        static constexpr size_t hb_size = vp_size + pad_bytes + block_size;
        static constexpr size_t bytes_allocated = hb_size * blocks;

    protected:

        static constexpr size_t b_size = block_size;
        
    public:
            
        /**
         * @brief Construct a new Memory Allocator object.
         * 
         */
        MemoryAllocator() : data_(nullptr), free_list_(nullptr)
        {
            data_ = new uchar[bytes_allocated];

            std::memset(data_, 0, bytes_allocated);

            for (size_t i = 0; i < blocks; ++i)
            {
                std::memset(data_ + vp_size + (i * hb_size), Pattern::UNALLOCATED, pad_bytes);
                push_list(reinterpret_cast<List*>(data_ + (i * hb_size)));
            }
        }

        /**
         * @brief Destructor
         * 
         */
        ~MemoryAllocator() noexcept
        {
            delete [] data_;
        }

        /**
         * @brief Allocates memory with O(1) performance.
         * 
         * @return void* 
         */
        void* Allocate()
        {
            if (!free_list_) throw std::runtime_error("Out of blocks.");

            uchar* memory = reinterpret_cast<uchar*>(free_list_) + vp_size;

            // safety check
            for (size_t i = 0; i < pad_bytes; ++i)
                if (memory[i] != Pattern::UNALLOCATED)
                    throw std::runtime_error("Corrupted block detected!");

            pop_list();

            std::memset(memory, Pattern::ALLOCATED, pad_bytes);

            return memory + pad_bytes;
        }

        /**
         * @brief Frees memory.
         * 
         * @param block 
         */
        void Free(void* block) noexcept
        {
            if (!block) std::abort();

            uchar* mem = reinterpret_cast<uchar*>(block) - pad_bytes;

            // safety check
            for (size_t i = 0; i < pad_bytes; ++i)
                if (mem[i] != Pattern::ALLOCATED)
                    std::abort();

            std::memset(mem, Pattern::UNALLOCATED, pad_bytes);

            push_list(reinterpret_cast<List*>(mem - vp_size));
        }

        /**
         * @brief Whether or not there is room for more allocations.
         * 
         * @return true 
         * @return false 
         */
        bool CanAllocate() const noexcept
        {
            return free_list_;
        }
        
        // prevent copying of any kind
        MemoryAllocator& operator=(MemoryAllocator& rhs) = delete;
        MemoryAllocator(const MemoryAllocator& rhs) = delete;
        MemoryAllocator(MemoryAllocator&& rhs) = delete;

    private:

        /**
         * @brief Pushes into internal list.
         * 
         * @param list 
         */
        void push_list(List* list) noexcept
        {
            list->next = free_list_;
            free_list_ = list;
        }

        /**
         * @brief Pops from front.
         * 
         */
        void pop_list() noexcept
        {
            free_list_ = free_list_->next;
        }
    };

    /**
     * @brief Works with the MemoryAllocator to allocate for types.
     * 
     * @tparam T 
     * @tparam blocks 
     */
    template <typename T, size_t blocks>
    struct TypeAllocator : public MemoryAllocator<alignof(T) + sizeof(T), blocks>
    {
        static_assert(!std::is_same<T, void>::value, "Cannot allocate type void.");

        /**
         * @brief Allocates and constructs object.
         * 
         * @tparam Args 
         * @param args 
         * @return T* 
         */
        template <typename... Args>
        T* Allocate(Args&&... args)
        {
            // get raw memory
            void* aligned_storage = AlignCheck(this->::Allocate());
            if (!aligned_storage) throw std::runtime_error("Allocation failed.");
            return new(aligned_storage) T(args...);
        }

        /**
         * @brief Frees memory block.
         * 
         * @param block 
         */
        void Free(T* block) noexcept
        {
            // safety check
            if (!block) std::abort();
            block->~T();
            this->::Free(block);
        }

        /**
         * @brief Ensures alignment safety.
         * 
         * @param memory 
         * @return void* 
         */
        void* AlignCheck(void* memory) const noexcept
        {
            size_t s = this->b_size;
            return std::align(alignof(T), sizeof(T), memory, s);
        }
    };
}
