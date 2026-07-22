#ifndef __RAII_H__
#define __RAII_H__

#include "oop.h"
typedef struct sptr_priv sptr_priv;

typedef struct sptr_t sptr_t;
struct sptr_t
{
    sptr_priv *priv;
    method(sptr_t *, borrow, void);
    method(void *, get_ptr, void);
};

void _SPTR_CLEAN_FUNCTION_CALLBACK_DONT_USE_IT_AS_A_FUNCTION(sptr_t **this);

ctor_decl(sptr_t, void *ptr);
dtor_decl(sptr_t);

typedef sptr_t *sptr_ptr_t;

#define make_shared(T, ...) new(sptr_t, new(T __VA_OPT__(,) __VA_ARGS__))

#define sptr                                                                                       \
    [[gnu::cleanup(_SPTR_CLEAN_FUNCTION_CALLBACK_DONT_USE_IT_AS_A_FUNCTION)]] sptr_ptr_t
#endif