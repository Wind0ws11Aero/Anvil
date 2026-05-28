#ifndef OOPCLANG_H
#define OOPCLANG_H

#include <Block.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "base.c"

#ifndef Block_copy
#error "oopclang.h requires Blocks support. Compile with clang -fblocks."
#endif

#define fptr ^

#define class(name)                                                            \
  typedef struct name name;                                                                                                                  \
  struct name

#define generic_class_init(name) add_class(#name)

#define method(ret, name, ...) ret (^name)(__VA_ARGS__)

#define extends(type) type base

#define lbd_t(ret, args) typeof({ret (^a) args; a})

#define lambda(ret, args, body) ({ret (^lbdfn) args = ^ret args body; lbdfn;})

#define bind(self, member, lbd)                                                \
  do {                                                                         \
    if ((self)->member) {                                                      \
      Block_release((self)->member);                                           \
    }                                                                          \
    (self)->member = Block_copy(lbd);                                 \
  } while (0)

#define unbind(self, member)                                                   \
  do {                                                                         \
    if ((self)->member) {                                                      \
      Block_release((self)->member);                                           \
      (self)->member = NULL;                                                   \
    }                                                                          \
  } while (0)

// #define ctor(name, ...) int name##_init(name *this __VA_OPT__(, )
// __VA_ARGS__)
#define ctor(name, ...)                                                        \
  int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__) =                 \
      ^int(name * this __VA_OPT__(, ) __VA_ARGS__)
#define ctor_decl(name, ...)                                                   \
  extern int (^name##_init)(name * this __VA_OPT__(, ) __VA_ARGS__)
#define getctor(name) (name##_init)
#define dtor(name)                                                                                                                                      \
  void (^name##_destroy_generic)(name * this) = ^(name * this)

#define dtor_decl(name)                                                        \
  extern void (^name##_destroy_generic)(void *this)

#define getdtor(name) (void (^)(void * this))(name##_destroy_generic)

#define new(name, ...)                                                         \
  ({                                                                           \
    name *oop_this__ = (calloc(1, sizeof(object_t) + sizeof(name)) + sizeof(object_t));                                   \
    object_t *bthis = ((void *)oop_this__) - sizeof(object_t);\
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
      object_t *bthis = (object_t *)((char *)oop_this__ - sizeof(object_t));\
      free(bthis);                                                        \
    }                                                                          \
  } while (0)

#endif