#include "../oop.h"
#include <stdlib.h>

#define pair(T, U)                                                             \
  class(__pair_##T##U) {                                                       \
    T first;                                                                   \
    U second;                                                                  \
  }                                                                          \

#define init_pair(this, first_, second_) ({this->first = first_; this->second = second_;})

#define new_pair(T, U, first, second) ({pair(T, U) *this = malloc(sizeof(struct __pair_##T##U)); init_pair(this, first, second); this;})

#define delete_pair(this) ({free(this);})
