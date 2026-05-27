#include <stdio.h>
#include "oop.h"
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
    do
{
    if ((this)->get_a)
    {
        _Block_release((const void *)((this)->get_a));
    }
    (this)->get_a = ((__typeof(({ int ( ^ lbdfn ) ( void ) = ^ int ( void ) { return this -> a ; } ; lbdfn ; })))_Block_copy((const void *)(({ int ( ^ lbdfn ) ( void ) = ^ int ( void ) { return this -> a ; } ; lbdfn ; }))));
} while (0);
    bind(this, get_b, lambda(float, (void), {return this->b;}));
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