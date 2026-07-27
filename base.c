// This is a part of oop.h, shouldn't be included manually
#include <stddef.h>

typedef struct Object Object;
struct Object
{
    const char *cls_name;
    void (*dtor_fn)(void *this);
    size_t capacity;
};
typedef Object object_t;

typedef void *ObjectCast;

#define to_objcast(var) ((ObjectCast)(var))