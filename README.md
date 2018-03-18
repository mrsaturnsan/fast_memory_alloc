# Fast Memory Allocator

Ready-to-use fixed-size C++ memory allocator.

## Features

- Safe!
- Fast!
- Only allocates memory from the OS once during construction!
- O(1) allocation and deallocation!

## Basic Usage
    #include "memoryallocator.h"
    
    // allocate 128 blocks of 32 bytes
    ATL::MemoryAllocator<32, 128> allocator;

    int* i = static_cast<int*>(allocator.Allocate());
    allocator.Free(i);

## Using with a class

    #include "memoryallocator.h"

    class MyClass
    {
        int i_;
        float f_;
        double d_;

        public:
            static ATL::TypeAllocator<MyClass, 128> alloc;

            // overload new and delete operators
            GEN_CLASS_NEW_DEL(alloc)

            MyClass(int i, float f, double d) noexcept : i_(i), f_(f), d_(d) {}
    };

    ATL::TypeAllocator<MyClass, 128> MyClass::alloc;

    MyClass* mc = new MyClass(1, 2, 3);
    delete mc;

## Requirements

- C++ 17 compliant compiler
