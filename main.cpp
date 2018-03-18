#include <iostream>
#include <string>

#include "memoryallocator.h"

int main()
{
    ATL::MemoryAllocator<32, 128> alloc;

    int* mem = alloc.Allocate<int>(1);

    int* mem2 = alloc.Allocate<int>(2);

    *mem = 5;

    alloc.Free(mem);

    int* mem3 = allo

    std::cout << *mem2 << '\n';

    return 0;
}