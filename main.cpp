#include "memoryallocator.h"

#include <iostream>

int main()
{
    ATL::TypeAllocator<int, 100> ta;

    int* i = ta.Allocate(0);
    ta.Free(i);
    
    return 0;
}
