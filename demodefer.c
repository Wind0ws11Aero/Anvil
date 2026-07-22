#include "defer.h"
#include <stdio.h>
#include <stdlib.h>
#include "fstream.h"
#include "oop.h"

int main()
{
    int *arr = malloc(sizeof(int) * 10);
    defer
    {
        printf("free\n");
        free(arr);
    };

    fstream *f = new(fstream, "defer_test.txt", "w");
    defer
    {
        printf("fclose\n");
        f->close();
    };

    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
        fprintf(f->raw_file, "arr[%d] = %d\n", i, arr[i]);
    }

    printf("arr[5] = %d\n", arr[5]);
}
