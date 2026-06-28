#include "fstream.h"
#include "oopclang.h"
#include "stdio.h"

int main(void)
{
    fstream *f = new(fstream, "a.txt", "w");
    f
        ->write("shit")
        ->write("fuck")
        ->flush();
    f->close();
    delete(fstream, f);
}