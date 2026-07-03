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

#define private(cls, membs...) struct __##cls##priv membs *priv

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

#endif