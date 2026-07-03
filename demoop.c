#include <stdio.h>
#include "oopclang.h"
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
    bind(this, get_a, void)
    {
        return this->a;
    };
    bind(this, get_b)
    {
        return this->b;
    };
    return 0;
};

dtor(A)
{
    unbind(this, get_a);
    unbind(this, get_b);
    printf("released obj\n");
};

void test1(void)
{
    A *a = new(A, 1, 2.0f);
    printf("manually manage memory: a->a = %d, a->b = %f\n", a->get_a(), a->get_b());
    printf("Is a instance of A? %d\n", instanceof(A, a));
    delete(a);
}
void test2(void)
{
    sptr a = new(sptr_t, new(A, 1, 2.0f));
    A *obj = a->get_ptr();
    printf("automaticlly manage memory: a->a = %d, a->b = %f\n", obj->get_a(), obj->get_b());
}

int main(void)
{
    test1();
    test2();
}