#include "memoryallocator.h"

int main()
{
    ATL::MemoryAllocator<32, 128> alloc;

    auto* mem = alloc.Allocate();
    alloc.Free(mem);
    
    return 0;
}