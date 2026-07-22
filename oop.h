#ifndef OOP_H
#define OOP_H

// Pragmas
#pragma GCC diagnostic ignored "-Wmissing-declarations"

// Impls
#include "flags.h"
#include "utils.h"
#include "base.c"
#include <Block.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef Block_copy
#error "oop.h requires Blocks support. Compile with clang -fblocks."
#endif

#define class(name)                                                                                \
    dtor_decl(name);                                                                               \
    typedef struct name name;                                                                      \
    struct name

#define interface(name)                                                                            \
    typedef struct name name;                                                                      \
    struct name

#define method(ret, name, ...) ret (^name)(__VA_ARGS__)

#define extends(type) type base

#define lbd_t(ret, args)                                                                           \
    typeof(({                                                                                      \
        ret(^a) args;                                                                              \
        a                                                                                          \
    }))

#define lambda(ret, args, body)                                                                    \
    ({                                                                                             \
        ret(^lbdfn) args = ^ret args body;                                                         \
        lbdfn;                                                                                     \
    })

#if !EXPER_BIND
#define bind(self, member, lbd)                                                                    \
    ({                                                                                             \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
        }                                                                                          \
        (self)->member = Block_copy(lbd);                                                          \
    })
#else
#define bind(self, member, ...)                                                                    \
    for (typeof((self)->member) __blck = (void *)1; __blck != NULL;                                \
         (self)->member = Block_copy(__blck), __blck = NULL)                                       \
    __blck = ^(__VA_ARGS__)
#endif
#define unbind(self, member)                                                                       \
    ({                                                                                             \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
            (self)->member = NULL;                                                                 \
        }                                                                                          \
    })

#define ctor(name, ...)                                                                            \
    int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__) =                                   \
        ^int(name * this __VA_OPT__(, ) __VA_ARGS__)
#define ctor_decl(name, ...) extern int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__)
#define getctor(name) (name##_init)
#define dtor(name)                                                                                 \
    void (^name##_destroy_generic)(void *this) = ^(void *this) {                                   \
      extern void (^name##_destroy)(name * this);                                                  \
      name##_destroy((name *)this);                                                                \
    };                                                                                             \
    void (^name##_destroy)(name * this) = ^(name * this)
#define dtor_decl(name) extern void (^name##_destroy_generic)(void *this)

#define getdtor(name) (void (^)(void *this))(name##_destroy_generic)

#define sizeof_object(obj) (sizeof(obj) + sizeof(Object))

#define adel [[gnu::cleanup(_cleanup_DD)]]

#define private(cls)                                                                               \
    struct __##cls##priv *priv;                                                                    \
    struct __##cls##priv

#define private_decl(cls) struct __##cls##priv *priv;

#define new(type, ...)                                                                             \
    ({                                                                                             \
        type *oop_this__ = (malloc(sizeof_object(type)) + sizeof(Object));            \
        Object *bthis = ((void *)oop_this__) - sizeof(Object);                                     \
        bthis->cls_name = #type;                                                                   \
        bthis->dtor_fn = type##_destroy_generic;                                                   \
        bthis->capacity = sizeof_object(type);\
        if (bthis && type##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0)                      \
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
            Object *bthis = (Object *)((char *)oop_this__ - sizeof(Object));                       \
            free(bthis);                                                                           \
        }                                                                                          \
    } while (0)
#else
#define delete(obj)                                                                                \
    ({                                                                                             \
        void *oop_this__ = (obj);                                                                  \
        if (oop_this__)                                                                            \
        {                                                                                          \
            Object *bthis = (Object *)((char *)oop_this__ - sizeof(Object));                       \
            bthis->dtor_fn(oop_this__);                                                            \
            free(bthis);                                                                           \
        }                                                                                          \
    })
#endif
void _cleanup_DD(void *v)
{
    delete(*(void **)v);
}
#define to_object(obj)                                                                             \
    ({                                                                                             \
        Object *oop_this__ = ((Object *)obj) - 1;                                                        \
        oop_this__;                                                                                      \
    })

#define to_class(clz, obj)                                                                         \
    ({  _macro_type_restriction(obj, Object *);                                                                                             \
        void *this = obj + 1;                                                                      \
        (clz *)this;                                                                               \
    })

#define instanceof(type, obj)                                                                      \
    ({                                                                                             \
        Object *this = to_object(obj);                                                             \
        strcmp(this->cls_name, #type) == 0;                                                        \
    })



#define __dcast(var) (void *)(var)


#define divfn_cast(base_t, div_t, fn, ...) (div_t *)(fn(EXPAND_ALL(__dcast, __VA_ARGS__)))
#define null nullptr

#endif