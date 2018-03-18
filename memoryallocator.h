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

// macros to make integration easier if making a static class allocator
#define CREATE_CLASS_NEW(_alloc_name) void* operator new(std::size_t) {return _alloc_name.MemoryAllocator::Allocate();}
#define CREATE_CLASS_DELETE(_alloc_name) void operator delete(void* ptr) {_alloc_name.MemoryAllocator::Free(ptr);}
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

        // meta data
        static constexpr size_t pad_bytes = 3;
        static constexpr size_t vp_size = sizeof(void*);
        static constexpr size_t hb_size = vp_size + pad_bytes + block_size;
        static constexpr size_t bytes_allocated = hb_size * blocks;

        // represents a list object
        struct List
        {
            List* next = nullptr;
        };

        // internal memory block
        uchar* data_;
        // page of available blocks
        List* free_list_;
        
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
            template <typename T = void, typename... Args>
            T* Allocate(Args&&... args)
            {
                if constexpr (std::is_same<T, void>::value)
                {
                    static_assert(sizeof...(Args) == 0, "Cannot initialize type void with arguments.");
                } 
                else
                {
                    static_assert((alignof(T) + sizeof(T)) <= block_size, "Block cannot hold T.");
                }

                if (!free_list_)
                    return nullptr;

                uchar* memory = reinterpret_cast<uchar*>(free_list_) + vp_size;

                // safety check
                for (size_t i = 0; i < pad_bytes; ++i)
                    if (memory[i] != Pattern::UNALLOCATED)
                        throw std::runtime_error("Corrupted block detected!");

                pop_list();

                std::memset(memory, Pattern::ALLOCATED, pad_bytes);

                if constexpr (std::is_same<T, void>::value)
                {
                    return (memory + pad_bytes);
                }
                else
                {
                    return new(memory + pad_bytes) T(args...);
                }
            }

            /**
             * @brief Frees memory.
             * 
             * @param block 
             */
            void Free(void* block)
            {
                if (!block) throw std::runtime_error("Attemping to free nullptr!");

                uchar* mem = reinterpret_cast<uchar*>(block) - pad_bytes;

                // safety check
                for (size_t i = 0; i < pad_bytes; ++i)
                    if (mem[i] != Pattern::ALLOCATED)
                        throw std::runtime_error("Freeing corrupted block!");

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
     * @brief Works with the MemoryAllocator to allocate for classes.
     * 
     * @tparam T 
     * @tparam blocks 
     */
    template <typename T, size_t blocks>
    struct ClassAllocator : public MemoryAllocator<sizeof(T) + alignof(T), blocks> {};
}