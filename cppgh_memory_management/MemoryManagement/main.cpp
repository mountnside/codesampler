
//This file/program contains a few examples that show possible usages
//of C++'s new and delete operators.
#include <memory>

extern void newAndDeleteArrayTest();
extern void placementNewTest();
extern void operatorNewTest();
extern void allocatorSTLTest();
extern void referenceCountingTest();
extern void smartPointerTest();
extern void memoryTrackingTest();

int main()
{
    char* nullPtr = NULL;
    
    //First of all, it is PERFECTLY LEGAL to delete a NULL pointer
    //(not so with C run-time free(), by the way).
    //There is NO need to check it for not being equal to NULL.
    delete nullPtr;

    newAndDeleteArrayTest();

    placementNewTest();

    operatorNewTest();

    allocatorSTLTest();

    referenceCountingTest();

    smartPointerTest();

    memoryTrackingTest();

    return 0;
}