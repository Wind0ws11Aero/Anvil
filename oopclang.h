#ifndef OOPCLANG_H
#define OOPCLANG_H

#include <Block.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef Block_copy
#error "oopclang.h requires Blocks support. Compile with clang -fblocks."
#endif

const char *__classes[10000] = {};
int __count = 0;
bool find_class(const char *clz) {
  for (int i = 0; i < __count; i++) {
    if (strcmp(__classes[i], clz) == 0) {
      return true;
    }
  }
  return false;
}

void add_class(const char *clz) { __classes[++__count] = clz; }

#define class(name)                                                            \
  typedef struct name name;                                                                                                                  \
  struct name

#include "base.c"

#define generic_class_init(name) add_class(#name)

#define method(ret, name, ...) ret (^name)(__VA_ARGS__)

#define extends(type) type base
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
#define dtor(name)                                                             \
  void (^name##_destroy_generic)(void *this) = ^(void *this) {                 \
    void (^name##_destroy)(name * this);                                       \
    name##_destroy((name *)this);                                              \
  };                                                                           \
  void (^name##_destroy)(name * this) = ^(name * this)

#define dtor_decl(name)                                                        \
  extern void (^name##_destroy)(name * this);                                  \
  extern void (^name##_destroy_generic)(void *this)

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