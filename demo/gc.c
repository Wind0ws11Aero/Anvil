#define TGC_IMPL
#include "../tgc.h"
#define NEWDEL_BYGC 1
#include <stdio.h>
#include <string.h>
#include "../oop.h"


class (A)
{
    int a;
    float b;
};

ctor(A, int a, float b)
{
    this->a = a;
    this->b = b;
    return 0;
}

dtor(A)
{
    puts("Object is freeing...");
    printf("%d", this->a);
}

A *a()
{
    A *obj = new(A, 1, 2.0f);
    return obj;
}

int main()
{
    A *obj = a();
    puts("obj has fetched from function a().");
}