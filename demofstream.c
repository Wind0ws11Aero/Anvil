#include "fstream.h"
#include "oop.h"

int main()
{
    adel fstream *f = new(fstream, "a.txt", "w");
    f
        ->write("Hello, ")
        ->write("world!")
        ->flush();
    f->close();
}