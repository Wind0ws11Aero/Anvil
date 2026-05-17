#ifndef __RAII_H__
#define __RAII_H__
#include "oop.h"
typedef struct sptr_priv sptr_priv;

typedef struct sptr_t sptr_t;
typedef struct sptr_t
{
    sptr_priv *priv;
    method(sptr_t *, borrow, void);
} sptr_t;
void _SPTR_CLEAN_FUNCTION_CALLBACK_DONT_USE_IT_AS_A_FUNCTION(sptr_t **this);
ctor(sptr_t, void *ptr, void (*del_fn)(void *));

typedef sptr_t *sptr_ptr_t;

#define sptr                                                                                       \
    __attribute__((cleanup(_SPTR_CLEAN_FUNCTION_CALLBACK_DONT_USE_IT_AS_A_FUNCTION))) sptr_ptr_t
#endif