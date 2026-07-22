#include "oop.h"
#include "stli.h"

class (String)
{
    extends(_stl);
    private(String)
    {
        char *c_str;
        size_t length;
    };
    method(char *, c_str, void);
    method(size_t, length, void);
    method(size_t, size, void);
    method(char *, begin, void);
    method(char *, end, void);
    method(char *, at, int);
};

ctor_decl(String, const char *cstr);
dtor_decl(String);