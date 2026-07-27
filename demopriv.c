#include "oop.h"

#define private(cls) struct __##cls##priv *priv; struct __##cls##priv

class (A)
{
    private (A)
    {
        int a;
        float b;
    };
    method(int, get_a);
    method(float, get_b);
};

ctor(A, int a, float b)
{
    alloc_priv(A);
    this->priv->a = a;
    this->priv->b = b;
    bind(this, get_a)
    {
        return this->priv->a;
    };
    bind(this, get_b)
    {
        return this->priv->b;
    };

    return 0;
};