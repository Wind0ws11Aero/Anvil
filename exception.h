#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "oop.h"
#include <setjmp.h>
#include <stdlib.h>

typedef struct Exception
{
    unsigned long long int errno;
    // A jmp_buf for setjmp and longjmp. Don't access it.
    jmp_buf *jbuf;
    const char *msg;
} Exception;

typedef struct ExceptionHandler
{
    Exception *list;
    size_t len;
} ExceptionHandler;

thread_local ExceptionHandler _handler;

thread_local Exception *_placeholder;

[[gnu::constructor]] void _ct()
{
    _handler.len = 0;
    _handler.list = malloc(sizeof(Exception));
    _placeholder = malloc(sizeof(Exception));
}

[[gnu::destructor]] void _dt()
{
    free(_handler.list);
    free(_placeholder);
}

void ExceptionHandler_push(ExceptionHandler *this, Exception e)
{
    ++this->len;
    this->list = realloc(this->list, sizeof(Exception) * this->len);
    this->list[this->len - 1] = e;
}

Exception *ExceptionHandler_pop(ExceptionHandler *this)
{
    Exception *e = &this->list[--this->len];
    this->list = realloc(this->list, sizeof(Exception) * (this->len ? this->len : 1));
    return e;
}

Exception *ExceptionHandler_peek(ExceptionHandler *this)
{
    return &this->list[this->len - 1];
}

#define try                                                                                        \
    {                                                                                              \
        jmp_buf __jb;                                                                              \
        int e = setjmp(__jb);                                                                      \
        if (e == 0)                                                                                \
            ExceptionHandler_push(&_handler, (Exception){.errno = 0, .jbuf = &__jb});              \
        if (e == 0)

#define catch                                                                                      \
    }                                                                                              \
    if (ExceptionHandler_peek(&_handler)->errno == 0) ExceptionHandler_pop(&_handler);\
    else for (Exception *__exception = ExceptionHandler_peek(&_handler);                                 \
         __exception->errno != 0 && __exception != _placeholder; __exception = _placeholder, ExceptionHandler_pop(&_handler))

#define exception_as(ename) Exception *ename = __exception

#define Exception(msg_) ({Exception *e = malloc(sizeof(Exception)); e->msg = msg_; e->errno = 1; e;})

[[noreturn]] void throw(Exception *e)
{
    Exception *oe = ExceptionHandler_peek(&_handler);
    oe->errno = e->errno;
    oe->msg = e->msg;
    free(e);
    longjmp(*oe->jbuf, 1);
}

// Annotation Hint Start
#define throws

#define noexcept
// Annotaion Hint End

#endif