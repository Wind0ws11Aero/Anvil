#ifndef OOPCLANG_H
#define OOPCLANG_H

#include "flags.h"

#include "base.c"
#include <Block.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef Block_copy
#error "oopclang.h requires Blocks support. Compile with clang -fblocks."
#endif

#define fptr ^

#define class(name)                                                                                \
    dtor_decl(name);\
    typedef struct name name;                                                                      \
    struct name

#define interface(name)                                                                                \
    typedef struct name name;                                                                      \
    struct name

#define generic_class_init(name) add_class(#name)

#define method(ret, name, ...) ret (^name)(__VA_ARGS__)

#define extends(type) type base

#define lbd_t(ret, args)                                                                           \
    typeof(({                                                                                       \
        ret (^a) args;                                                                              \
        a                                                                                          \
    }))

#define lambda(ret, args, body)                                                                    \
    ({                                                                                             \
        ret (^lbdfn) args = ^ret args body;                                                         \
        lbdfn;                                                                                     \
    })

#if !EXPER_BIND
#define bind(self, member, lbd)                                                                    \
    ({                                                                                            \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
        }                                                                                          \
        (self)->member = Block_copy(lbd);                                                          \
    })
#else
#define bind(self, member, ...) for (typeof((self)->member) __blck = (void *)1; __blck != NULL; (self)->member = Block_copy(__blck), __blck = NULL) __blck = ^(__VA_ARGS__)
#endif
#define unbind(self, member)                                                                       \
    ({                                                                                              \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
            (self)->member = NULL;                                                                 \
        }                                                                                          \
    })

// #define ctor(name, ...) int name##_init(name *this __VA_OPT__(, )
// __VA_ARGS__)
#define ctor(name, ...)                                                                            \
    int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__) =                                   \
        ^int(name * this __VA_OPT__(, ) __VA_ARGS__)
#define ctor_decl(name, ...) extern int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__)
#define getctor(name) (name##_init)
#define dtor(name)                                                             \
  void (^name##_destroy_generic)(void *this) = ^(void *this) {               \
    extern void (^name##_destroy)(name * this);                                       \
    name##_destroy((name *)this);                                            \
  };                                                                           \
  void (^name##_destroy)(name * this) = ^(name * this)
#define dtor_decl(name) extern void (^name##_destroy_generic)(void *this)

#define getdtor(name) (void (^)(void *this))(name##_destroy_generic)

#define private(cls) struct __##cls##priv *priv; struct __##cls##priv

#define new(name, ...)                                                                             \
    ({                                                                                             \
        name *oop_this__ = (calloc(1, sizeof(Object) + sizeof(name)) + sizeof(Object));        \
        Object *bthis = ((void *)oop_this__) - sizeof(Object);                                 \
        bthis->cls_name = #name;                                                                   \
        bthis->dtor_fn = name##_destroy_generic;\
        if (bthis && name##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0)                      \
        {                                                                                          \
            abort();                                                                               \
            oop_this__ = NULL;                                                                     \
        }                                                                                          \
        oop_this__;                                                                                \
    })

#if !EXPER_DELETE
#define delete(name, obj)                                                                          \
    do                                                                                             \
    {                                                                                              \
        name *oop_this__ = (obj);                                                                  \
        if (oop_this__)                                                                            \
        {                                                                                          \
            name##_destroy_generic(oop_this__);                                                    \
            Object *bthis = (Object *)((char *)oop_this__ - sizeof(Object));                 \
            free(bthis);                                                                           \
        }                                                                                          \
    } while (0)
#else
#define delete(obj)                                                                          \
    ({                                                                                              \
        void *oop_this__ = (obj);                                                                  \
        if (oop_this__)                                                                            \
        {                                                                                          \
            Object *bthis = (Object *)((char *)oop_this__ - sizeof(Object));                 \
            bthis->dtor_fn(oop_this__); free(bthis);                                                                           \
        }                                                                                          \
    })
#endif

#define to_object(obj)                                                                             \
    ({                                                                                             \
        Object *this = ((Object *)obj) - 1;                                                    \
        this;                                                                                      \
    })

#define instanceof(type, obj)                                                                      \
    ({                                                                                             \
        Object *this = to_object(obj);                                                           \
        strcmp(this->cls_name, #type) == 0;                                                                   \
    })


#define __dcast(var) (void *)(var)

#define EXPAND_1(func, var) func(var)
#define EXPAND_2(func, var, ...) func(var), EXPAND_1(func, __VA_ARGS__)
#define EXPAND_3(func, var, ...) func(var), EXPAND_2(func, __VA_ARGS__)
#define EXPAND_4(func, var, ...) func(var), EXPAND_3(func, __VA_ARGS__)
#define EXPAND_5(func, var, ...) func(var), EXPAND_4(func, __VA_ARGS__)
#define EXPAND_6(func, var, ...) func(var), EXPAND_5(func, __VA_ARGS__)
#define EXPAND_7(func, var, ...) func(var), EXPAND_6(func, __VA_ARGS__)
#define EXPAND_8(func, var, ...) func(var), EXPAND_7(func, __VA_ARGS__)
#define EXPAND_9(func, var, ...) func(var), EXPAND_8(func, __VA_ARGS__)
#define EXPAND_10(func, var, ...) func(var), EXPAND_9(func, __VA_ARGS__)
#define EXPAND_11(func, var, ...) func(var), EXPAND_10(func, __VA_ARGS__)
#define EXPAND_12(func, var, ...) func(var), EXPAND_11(func, __VA_ARGS__)
#define EXPAND_13(func, var, ...) func(var), EXPAND_12(func, __VA_ARGS__)
#define EXPAND_14(func, var, ...) func(var), EXPAND_13(func, __VA_ARGS__)
#define EXPAND_15(func, var, ...) func(var), EXPAND_14(func, __VA_ARGS__)
#define EXPAND_16(func, var, ...) func(var), EXPAND_15(func, __VA_ARGS__)

// clang-format off
#define SELECT( \
  _1, _2, _3, _4, \
  _5, _6, _7, _8, \
  _9, _10, _11, _12, \
  _13, _14, _15, _16, \
  name, ...) name
// clang-format on

#define EXPAND_ALL(func, ...) __VA_OPT__( \
  SELECT(__VA_ARGS__, \
    EXPAND_16, EXPAND_15, EXPAND_14, EXPAND_13, \
    EXPAND_12, EXPAND_11, EXPAND_10, EXPAND_9, \
    EXPAND_8, EXPAND_7, EXPAND_6, EXPAND_5, \
    EXPAND_4, EXPAND_3, EXPAND_2, EXPAND_1 \
  )(func, __VA_ARGS__))

#define divfn_cast(base_t, div_t, fn, ...) (div_t *)(fn(EXPAND_ALL(__dcast, __VA_ARGS__)))

#endif