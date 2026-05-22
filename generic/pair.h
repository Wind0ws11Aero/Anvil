#include "../oop.h"
#include <stdlib.h>

#define pair(T, U)       \
    class(__pair_##T##U) \
    {                    \
        T first;         \
        U second;        \
    }

#define pair_init(this, first_, second_) ({this->first = first_; this->second = second_; })

#define new_pair(T, U, first, second) ({pair(T, U) *this = malloc(sizeof(struct __pair_##T##U)); pair_init(this, first, second); this; })

#define pair_destroy(this) ({free(this);})
