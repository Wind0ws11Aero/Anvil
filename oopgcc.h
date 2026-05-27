#ifndef OOPGCC_H
#define OOPGCC_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "base.c"


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
    name *oop_this__ = malloc(sizeof(name));                                   \
    if (oop_this__ &&                                                          \
        name##_init(oop_this__ __VA_OPT__(, ) __VA_ARGS__) != 0) {             \
      free(oop_this__);                                                        \
      oop_this__ = NULL;                                                       \
    }                                                                          \
    oop_this__;                                                                \
  })

#define delete(name, obj)                                                      \
  do {                                                                         \
    name *oop_this__ = (obj);                                                  \
    if (oop_this__) {                                                          \
      name##_destroy(oop_this__);                                              \
      free(oop_this__);                                                        \
    }                                                                          \
  } while (0)

#endif