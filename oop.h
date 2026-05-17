#ifndef OOP_H
#define OOP_H

#include <Block.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef __BLOCKS__
#error "oop.h requires Blocks support. Compile with clang -fblocks."
#endif

#define CLASS(name)                                                                                \
    typedef struct name name;                                                                      \
    struct name

#define class(name) CLASS(name)

#define method(ret, name, ...) ret (^name)(__VA_ARGS__)

#define extends(type) type base

#define bind(self, member, ...)                                                                    \
    do                                                                                             \
    {                                                                                              \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
        }                                                                                          \
        (self)->member = Block_copy(^__VA_ARGS__);                                                 \
    } while (0)

#define unbind(self, member)                                                                       \
    do                                                                                             \
    {                                                                                              \
        if ((self)->member)                                                                        \
        {                                                                                          \
            Block_release((self)->member);                                                         \
            (self)->member = NULL;                                                                 \
        }                                                                                          \
    } while (0)

#define ctor(name, ...) int name##_init(name *this __VA_OPT__(, ) __VA_ARGS__)
#define getctor(name) (name##_init)
#define dtor(name)                                                                                 \
    void name##_destroy(name *this);                                                               \
    void name##_destroy_generic(void *this)                                                        \
    {                                                                                              \
        name##_destroy((name *)this);                                                              \
    };                                                                                             \
    void name##_destroy(name *this)
#define getdtor(name) (name##_destroy_generic)

#define NEW(name, ...)                                                                             \
    ({                                                                                             \
        name *oop_this__ = malloc(sizeof(name));                                                   \
        if (oop_this__ && name##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0)                 \
        {                                                                                          \
            free(oop_this__);                                                                      \
            oop_this__ = NULL;                                                                     \
        }                                                                                          \
        oop_this__;                                                                                \
    })

#define DELETE(name, obj)                                                                          \
    do                                                                                             \
    {                                                                                              \
        name *oop_this__ = (obj);                                                                  \
        if (oop_this__)                                                                            \
        {                                                                                          \
            name##_destroy(oop_this__);                                                            \
            free(oop_this__);                                                                      \
        }                                                                                          \
    } while (0)

#define new NEW
#define delete DELETE
#endif