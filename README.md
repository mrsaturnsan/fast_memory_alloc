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

## Class/Struct Usage

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

## Test Cases

    #define SIZE 5000000

    struct Test
    {
        char c[128];
        float f;
        int i;
        unsigned u;
    };

    // Case 1
    ATL::MemoryAllocator<sizeof(Test) + alignof(Test), SIZE> atl;
    Test* tarr[SIZE];

    for (size_t i = 0; i < SIZE; ++i)
        tarr[i] = static_cast<Test*>(atl.Allocate());

    for (size_t i = 0; i < SIZE; ++i)
        atl.Free(tarr[i]);

    // Case 2
    for (size_t i = 0; i < SIZE; ++i)
        tarr[i] = new Test;

    for (size_t i = 0; i < SIZE; ++i)
        delete tarr[i];

    SIZE = 32 : 7 times faster
    SIZE = 64 : 8 times faster
    SIZE = 128 : 9 times faster
    SIZE = 256 : 8.29 times faster
    SIZE = 512 : 8.52 times faster
    SIZE = 1024 : 9 times faster
    SIZE = 2048 : 9.85 times faster
    SIZE = 1000000 : 4.8 times faster
    SIZE = 5000000 : 4.6 times faster
