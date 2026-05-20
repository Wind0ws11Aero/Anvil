#include <stdio.h>
#include "oop.h"
#define __RAII_H_DEBUG__
#include "raii.h"

class (A)
{
    int a;
    float b;
    method(int, get_a, void);
    method(float, get_b, void);
};

ctor(A, int a, float b)
{
    this->a = a;
    this->b = b;
    bind(this, get_a, {return this->a;});
    bind(this, get_b, {return this->b;});
    return 0;
};

dtor(A)
{
    unbind(this, get_a);
    unbind(this, get_b);
    printf("released obj");
};

void test1(void)
{
    A *a = new(A, 1, 2.0f);
    printf("manually manage memory: a->a = %d, a->b = %f\n", a->get_a(), a->get_b());
    delete(A, a);
}

void test2(void)
{
    sptr a = new(sptr_t, new(A, 1, 2.0f), getdtor(A));
    A *obj = a->get_ptr();
    printf("automaticlly manage memory: a->a = %d, a->b = %f\n", obj->get_a(), obj->get_b());
}

int main(void)
{
    test1();
    test2();
}