#ifndef REFLECT_H
#define REFLECT_H

#include "oop.h"
#include <stddef.h>
#include <string.h>

class(__reflect_member_t)
{
    const char *name;
    size_t offset;
};

class(__reflect_class_t)
{
    const char *cls_name;
    __reflect_member_t *members;
    int count;
};

static __reflect_class_t *__reflect_registry[1000];
static int __reflect_reg_count = 0;

static inline void __reflect_reg_class(__reflect_class_t *meta)
{
    __reflect_registry[__reflect_reg_count++] = meta;
}

static inline void *__reflect_get_ptr(void *obj, const char *name)
{
    if (!obj)
        return NULL;
    const char *actual_cls = to_object(obj)->cls_name;

    for (int i = 0; i < __reflect_reg_count; i++)
    {
        if (strcmp(__reflect_registry[i]->cls_name, actual_cls) == 0)
        {
            for (int j = 0; j < __reflect_registry[i]->count; j++)
            {
                if (strcmp(__reflect_registry[i]->members[j].name, name) == 0)
                {
                    return (void *)((char *)obj + __reflect_registry[i]->members[j].offset);
                }
            }
        }
    }
    return NULL;
}

#define rfl_member(cls, mem) {#mem, offsetof(cls, mem)}

#define reflect(cls, ...)                                                                          \
    static __reflect_member_t _m_##cls[] = {__VA_ARGS__};                                          \
    static __reflect_class_t _meta_##cls = {#cls, _m_##cls,                                        \
                                            sizeof(_m_##cls) / sizeof(__reflect_member_t)};        \
    __attribute__((constructor)) static void _init_##cls(void)                                     \
    {                                                                                              \
        __reflect_reg_class(&_meta_##cls);                                                         \
    }

#define getfield(cls, obj, mem) (*(typeof(((cls *)0)->mem) *)__reflect_get_ptr(obj, #mem))
#define setfield(cls, obj, mem, val) (getfield(cls, obj, mem) = (val))
#define callmeth(cls, obj, mem, ...) (getfield(cls, obj, mem)(__VA_ARGS__))

#endif