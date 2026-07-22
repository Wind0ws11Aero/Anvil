#include "anvilcomps.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "oop.h"
#include "stli.h"
#include <stdlib.h>

#define privn(cls) struct __##cls##priv

#define alloc_priv(cls) this->priv = malloc(sizeof(struct __##cls##priv))

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

ctor(String, const char *cstr)
{
    alloc_priv(String);
    this->priv->c_str = malloc(strlen(cstr));
    strcpy(this->priv->c_str, cstr);
    this->priv->length = strlen(cstr);
    bind(this, c_str)
    {
        return this->priv->c_str;
    };
    bind(this, length)
    {
        return this->priv->length;
    };
    bind(&this->base, size)
    {
        return this->length() + 1;
    };
    bind(&this->base, begin)
    {
        return (void *)this->c_str();
    };
    bind(&this->base, end)
    {
        return (void *)this->c_str() + this->length();
    };
    bind(&this->base, at, int sub)
    {
        if (sub >= this->length()) throw(Exception("index overlaped"));
        return (void *)(this->c_str() + sub);
    };
    bind(this, at, int sub)
    {
        return (char *)this->base.at(sub);
    };
    bind(this, begin)
    {
        return this->c_str();
    };
    bind(this, end)
    {
        return (char *)this->base.end();
    };
    bind(this, size)
    {
        return this->base.size();
    };
    return 0;
};

dtor(String)
{
    free(this->priv->c_str);
    free(this->priv);
    unbind(this, c_str);
    unbind(this, length);
    unbind(this, begin);
    unbind(this, end);
    unbind(this, size);
    unbind(this, at);
    unbind(&this->base, size);
    unbind(&this->base, begin);
    unbind(&this->base, end);
    unbind(&this->base, at);
};