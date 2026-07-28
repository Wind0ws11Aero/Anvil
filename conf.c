#include <stdio.h>

void *global;
[[gnu::constructor(0)]] void __a()
{
    global = __builtin_frame_address(0);
}

int main()
{
	printf("%ld", __builtin_frame_address(0) - global);
}
