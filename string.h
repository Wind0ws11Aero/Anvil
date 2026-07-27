#ifndef STRING_H
#define STRING_H

#include "oop.h"
#include "stli.h"

class (String)
{
    extends(_stl);
    private_decl(String);
    method(char *, c_str, void);
    method(size_t, length, void);
    method(size_t, size, void);
    method(char *, begin, void);
    method(char *, end, void);
    method(char *, at, int);
};

ctor_decl(String, const char *cstr);
dtor_decl(String);

#endif