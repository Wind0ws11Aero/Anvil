#ifndef STL_INTERFACE_H
#define STL_INTERFACE_H

#include "oop.h"

interface (_stl)
{
    method(size_t, size, void);
    method(void *, begin, void);
    method(void *, end, void);
    method(void *, at, int);
};

#endif