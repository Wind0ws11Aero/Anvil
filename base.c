/* This is a part of code in oop.h, shouldn't include it manually */

typedef struct Object Object;
struct Object
{
    const char *cls_name;
    void (^dtor_fn)(void *this);
};
typedef Object object_t;