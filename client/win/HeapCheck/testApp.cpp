#include <iostream.h>

// Always include "swaps.h" LAST, after all other includes you make
// (unless of course one of your includes invokes new/delete/malloc/etc)

#include "swaps.h"

// allocations deep in call stacks...f1() -> f2() -> f3()
void f3()
{
    char *p = new char[20];
}

void f2()
{
    f3();
}

void f1()
{
    f2();
}

main()
{
    char *p = new char[10];

    // allocations deep in call stacks...
    // f1();

    // Through malloc, too, not just new/delete...
    // char *q = (char *) malloc(200);

    // Read/write access beyond bounds
    // char a = p[10];
    
    // Access before bounds (globally define PRE_CHECK in project settings)
    // p[-1] = 0;

    // Forgetting to delete
    // delete [] p;

    // Reading from freed memory
    // char a = p[0];    
 
    // Deleting/freeing wrong pointer
    // free((char *)1);

    return 0;
}