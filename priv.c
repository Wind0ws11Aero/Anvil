#include "oop.h"

class (A)
{
    private(A, {
        int a;
        float b;
    });
    method(int, get_a);
    method(float, get_b);
};

ctor(A, int a, float b)
{
    this->priv->a = a;
    this->priv->b = b;
    bind(this, get_a, ^{return this->priv->a;});
    bind(this, get_b, ^{return this->priv->b;});

    return 0;
};