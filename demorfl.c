#include "oop.h"
#include "oopclang.h"
#include "reflect.h"
#include <stdio.h>

class (A)
{
    int a;
    method(int, get_a, void);
};

ctor(A, int a)
{
    this->a = a;
    bind(this, get_a)
    {
        return this->a;
    };
    return 0;
};

dtor(A) {};

reflect(
    A,
    rfl_member(A, a),
    rfl_member(A, get_a)
)

int main(void)
{
    void *a = new(A, 5);
    printf("%d\n", getfield(A, a, a));
    printf("%d\n", callmeth(A, a, get_a));
}