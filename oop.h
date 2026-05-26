#ifndef OOP_H
#define OOP_H
#ifdef __clang__
#include "oopclang.h"
#elif defined(__GNUC__)
#include "oopgcc.h"
#endif
#endif