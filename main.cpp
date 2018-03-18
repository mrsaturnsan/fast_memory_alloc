#include <iostream>
#include <string>

#include "memoryallocator.h"

int main()
{
    ATL::MemoryAllocator<32, 128> alloc;

    int* mem = alloc.Allocate<int>(1);
    alloc.Free(mem);
    
    return 0;
}