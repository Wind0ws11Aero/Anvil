#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "oop.h"
#include "oop.h"
#include <_abort.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class (Exception)
{
    unsigned long long int errno;
    // A jmp_buf for setjmp and longjmp. Don't access it.
    jmp_buf *jbuf;
    const char *msg;
};

#define Exception(msg_)                                                                            \
    ({                                                                                             \
        Exception *e = malloc(sizeof(Exception));                                                  \
        e->msg = msg_;                                                                             \
        e->errno = 1;                                                                              \
        e;                                                                                         \
    })

class (ExceptionHandler)
{
    Exception *list;
    size_t len;
    method(void, push, Exception);
    method(Exception *, pop, void);
    method(Exception *, peek, void);
};

ctor(ExceptionHandler)
{
    this->len = 0;
    this->list = malloc(sizeof(Exception));
    bind(this, push, Exception e)
    {
        ++this->len;
        this->list = realloc(this->list, sizeof(Exception) * this->len);
        this->list[this->len - 1] = e;
    };
    bind(this, pop)
    {
        Exception *e = &this->list[--this->len];
        Exception *ret = malloc(sizeof(Exception));
        memcpy(ret, e, sizeof(Exception));
        this->list = realloc(this->list, sizeof(Exception) * (this->len ? this->len : 1));
        return ret;
    };
    bind(this, peek)
    {
        return &this->list[this->len - 1];
    };
    return 0;
};

dtor(ExceptionHandler)
{
    free(this->list);
    unbind(this, pop);
    unbind(this, peek);
    unbind(this, push);
};

thread_local ExceptionHandler *_global_handler;

thread_local Exception *_placeholder;

[[gnu::constructor]] void _exception_init()
{
    _global_handler = new(ExceptionHandler);
    _global_handler->len = 0;
    _global_handler->list = malloc(sizeof(Exception));
    _placeholder = Exception("");
}

[[gnu::destructor]] void _exception_deinit()
{
    delete(_global_handler);
    free(_placeholder);
}

#define try_with_handler(_handler)                                                                                        \
    {                                                                                              \
        jmp_buf __jb;                                                                              \
        int e = setjmp(__jb);                                                                      \
        if (e == 0)                                                                                \
            _handler->push((Exception){.errno = 0, .jbuf = &__jb});               \
        if (e == 0)

#define catch_with_handler(_handler)                                                                                      \
    }                                                                                              \
    if (_handler->peek()->errno == 0)                                               \
        free(_handler->pop());                                                            \
    else                                                                                           \
        for (Exception *__exception = _handler->pop();                             \
             __exception->errno != 0 && __exception != _placeholder;                               \
             free(__exception), __exception = _placeholder)

#define try try_with_handler(_global_handler)
#define catch catch_with_handler(_global_handler)

#define exception_as(ename) Exception *ename = __exception

[[noreturn]] void throw_with_handler(ExceptionHandler *_handler, Exception * e)
{
    if (_handler->len == 0)
    {
        perror("ExceptionHandler: terminating due to an uncaught exception. Aborting.\n");
        abort();
    }
    Exception *oe = _handler->peek();
    oe->errno = e->errno; 
    oe->msg = e->msg;
    free(e);
    longjmp(*oe->jbuf, 1);
}

[[noreturn]] void throw(Exception *e)
{
    throw_with_handler(_global_handler, e);
}

// Annotation Hint Start
#define throws

#define noexcept
// Annotaion Hint End

#endif