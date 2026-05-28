#include "raii.h"
#include "oop.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#if __STDC_VERSION__ >= 202311L
#undef NULL
#define NULL (nullptr)
#endif

struct sptr_priv
{
    void *rptr;
    atomic_int refc;
    void (fptr del_fn)(void *);
};

struct sptr_t
{
    sptr_priv *priv;
    method(sptr_t *, borrow, void);
    method(void *, get_ptr, void);
};

void _SPTR_CLEAN_FUNCTION_CALLBACK_DONT_USE_IT_AS_A_FUNCTION(sptr_t **this)
{
    printf("In free, refc: %d\n", (*this)->priv->refc);
    if (atomic_fetch_sub(&((*this)->priv->refc), 1) == 1)
    {
        if ((*this)->priv->del_fn != NULL)
        {
            (*this)->priv->del_fn((*this)->priv->rptr);
        }
        free((char *)(*this)->priv->rptr - sizeof(object_t));

        free((void *)((char *)((*this)->priv) - sizeof(object_t)));
        free((char *)(*this) - sizeof(object_t));
    }
}

sptr_t *sptr_borrow_fn(sptr_t *this)
{
    atomic_fetch_add(&this->priv->refc, 1);
    return this;
}
ctor(sptr_priv)
{
    return 0;
};

ctor(sptr_t, void *ptr, void (fptr del_fn)(void *))
{
    this->priv = new(sptr_priv);
    this->priv->rptr = ptr;
    this->priv->del_fn = del_fn;
    atomic_init(&this->priv->refc, 1);
    bind(this, borrow, lambda(sptr_t *, (void), { return sptr_borrow_fn(this); }));
    bind(this, get_ptr, lambda(void *, (void), { return this->priv->rptr; }));
    return 0;
};
