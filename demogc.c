#include <string.h>
#define TGC_IMPL
#include "tgc.h"
#include <stdio.h>
#include <execinfo.h>

static tgc_t gc;

void d(void *)
{
    puts("FREE!");
}

#define gc_block for (int __oncegc = (tgc_start(&gc, __builtin_frame_address(0)), 0); __oncegc != 1; __oncegc++, tgc_stop(&gc))

void *global;

[[gnu::constructor(0)]] void __a()
{
    printf("frame[0]: %p\n", __builtin_frame_address(0));
    global = __builtin_frame_address(0);
    tgc_start(&gc, __builtin_frame_address(0) + 6320);
}

#ifndef _OFFSOFCAM
#error _OFFSOCAM is not defined which is important to gc, build with ./nob .
#endif

int main()
{
    printf("frame[0]: %p\n", __builtin_frame_address(0));
    printf("%ld\n", _OFFSOFCAM);
    char *a = tgc_alloc_opt(&gc, 6, 0, d);
    strcpy(a, "Hello");
    puts(a);
    tgc_stop(&gc);
}