/* This is a part of code in oop.h, wouldn't, can't include it manually */

typedef struct type_t type_t;
struct type_t
{
    const char *name;
    int typeid;
};

typedef struct object_t object_t;
struct object_t
{
    const type_t type;
};
