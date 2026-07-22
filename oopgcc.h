#ifndef OOPGCC_H
#define OOPGCC_H
#error Anvil will not support GCC.

// This is the legacy oopgcc.h code, it is broke, and it is deprecated. 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "base.c"

#define fptr *


#define class(name)                                                            \
  typedef struct name name;                                                                                                                  \
  struct name


#define generic_class_init(name) add_class(#name)

#define method(ret, name, ...) ret (*name)(__VA_ARGS__)

#define extends(type) type base

#define lbd_t(ret, args) typeof(({ret a args; a;}))

#define lambda(ret, args, body) ({ret lbdfn args body; lbdfn;})

#define bind(self, member, lbd)                                                \
  do {                                                                                                                                           \
    (self)->member = lbd;                                 \
  } while (0)

#define unbind(self, member)                                                   \
  do {                                                                         \
    if ((self)->member) {                                                                                                \
      (self)->member = NULL;                                                   \
    }                                                                          \
  } while (0)

#define ctor(name, ...)                                                        \
  int name##_init(name * this __VA_OPT__(, ) __VA_ARGS__)
#define ctor_decl(name, ...)                                                   \
  extern int name##_init(name * this __VA_OPT__(, ) __VA_ARGS__)
#define getctor(name) (name##_init)
#define dtor(name)                                                             \
  void name##_destroy_generic(void *this) {                 \
    void name##_destroy(name * this);                                       \
    name##_destroy((name *)this);                                              \
  };                                                                           \
  void name##_destroy(name * this) 

#define dtor_decl(name)                                                        \
  extern void name##_destroy(name * this);                                  \
  extern void name##_destroy_generic(void *this)

#define getdtor(name) (name##_destroy_generic)

#define new(name, ...)                                                         \
  ({                                                                           \
    name *oop_this__ = (calloc(1, sizeof(Object) + sizeof(name)) + sizeof(Object));                                   \
    Object *bthis = ((Object *)oop_this__) - 1;\
    bthis->cls_name = #name;\
    if (bthis &&                                                          \
        name##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0) {             \
      abort();                                                  \
      oop_this__ = NULL;                                                       \
    }                                                                          \
    oop_this__;                                                                \
  })

#define delete(name, obj)                                                      \
  do {                                                                         \
    name *oop_this__ = (obj);                                                  \
    if (oop_this__) {                                                          \
      name##_destroy_generic(oop_this__);                                              \
      Object *bthis = (Object *)((char *)oop_this__ - sizeof(Object));\
      free(bthis);                                                        \
    }                                                                          \
  } while (0)


#endif