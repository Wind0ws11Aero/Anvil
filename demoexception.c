#include "exception.h"
#include <stdio.h>

int main(void) throws
{
    try
    {
        throw(Exception("error\n"));
    }
    catch
    {
        exception_as(e);
        printf("%s", e->msg);
    }
    
}