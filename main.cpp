#include "memoryallocator.h"

#include <iostream>

struct MyStruct
{
    static ATL::TypeAllocator<MyStruct, 10> alloc;

    int i;
    int j;
    int k;

    MyStruct()
    {
        std::cout << "Constructed\n";
    }

    ~MyStruct()
    {
        std::cout << "Destructed\n";
    }

    GEN_CLASS_NEW_DEL(alloc)

};

ATL::TypeAllocator<MyStruct, 10> MyStruct::alloc;


int main()
{
    MyStruct* ms = new MyStruct;

    delete ms;

    
    return 0;
}
