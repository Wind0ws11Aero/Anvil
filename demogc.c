#include <string.h>
#define TGC_IMPL
#include "tgc.h"
#include <stdio.h>

static tgc_t gc;


void dtor(void *)
{
    puts("FREE!");
}

#define gc_block for (int __oncegc = (tgc_start(&gc, __builtin_frame_address(0)), 0); __oncegc != 1; __oncegc++, tgc_stop(&gc))

int main()
{
    gc_block
    {
        char *hello_str = tgc_alloc_opt(&gc, 6, 0, dtor);
        strcpy(hello_str, "Hello");
        printf("%s\n", hello_str);
    }
}