#ifndef DEFER_H
#define DEFER_H

#ifndef __STDC_DEFER_TS25755__
#include <Block.h>
#include "Block_private.h"

#define __DEFER_CONCAT_IMPL(a, b) a##b
#define __DEFER_CONCAT_HLPR(a, b) __DEFER_CONCAT_IMPL(a, b)
#define __DEFER_CONCAT(a, b) __DEFER_CONCAT_HLPR(a, b)

typedef void (^defer_block_t)(void);

static inline void _defer_cleanup_fn(defer_block_t *blk)
{
    if (*blk)
    {
        struct Block_layout *layout = (struct Block_layout *)(*blk);
        void (*invoke_fn)(void *) = (void (*)(void *))_Block_get_invoke_fn(layout);
        invoke_fn((void *)*blk);
    }
}

#define defer                                                                                       \
    __attribute__((cleanup(_defer_cleanup_fn)))                                                     \
    defer_block_t __DEFER_CONCAT(__defer_fn_, __COUNTER__) = ^

#else
#define defer _Defer
#endif
#endif