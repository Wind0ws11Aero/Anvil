#define TGC_IMPL
#include "../tgc.h"
#include <stdio.h>
#include <string.h>
#include "../oop.h"

static tgc_t gc;

[[gnu::constructor(0)]] void __initializer()
{
    tgc_start(&gc, __builtin_frame_address(0) + _OFFSOFCAM);
}

[[gnu::destructor]] void __deinitializer()
{
    tgc_stop(&gc);
}

int __counter_()
{
    static int i = 0;
    if (i == 3) i = 0;
    return ++i;
}

#define gnew(type, ...)                                                                             \
    ({                                                                                             \
        type *oop_this__ = (tgc_alloc_opt(&gc, sizeof_object(type), 0, getdtor_objh(type)) + sizeof(Object));            \
        Object *bthis = ((void *)oop_this__) - sizeof(Object);                                     \
        bthis->cls_name = #type;                                                                   \
        bthis->dtor_fn = type##_destroy_generic;                                                   \
        bthis->capacity = sizeof_object(type);\
        if (bthis && type##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0)                      \
        {                                                                                          \
            abort();                                                                               \
            oop_this__ = NULL;                                                                     \
        }                                                                                          \
        if (__counter_() == 3) tgc_sweep(&gc); \
        oop_this__;                                                                                \
    })

class (A)
{
    int a;
    float b;
};

ctor(A, int a, float b)
{
    this->a = a;
    this->b = b;
    return 0;
}

dtor(A)
{
    puts("Object is freeing...");
    printf("%d", this->a);
}

A *a()
{
    A *obj = gnew(A, 1, 2.0f);
    return obj;
}

int main()
{
    A *obj = a();
    puts("obj has fetched from functuon a().");
}