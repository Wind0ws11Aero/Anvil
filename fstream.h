#include <stdio.h>
#include "oop.h"
#include "exception.h"


class (fstream)
{
    FILE *raw_file;
    method(fstream *, write, char *);
    method(void, close, void);
    method(fstream *, flush, void);
    method(fstream *, rewind, void);
    method(fstream *, seek, long, int);
    method(long, tell, void);
};

dtor(fstream) {};

ctor(fstream, const char *fname, const char *mode) throws
{
    this->raw_file = fopen(fname, mode);
    if (this->raw_file == NULL) 
    {
        delete(this);
        throw(Exception("Failed to open file."));
    }
    bind(this, write, ^(char *cont){
        if (fputs(cont, this->raw_file) < 0) throw(Exception("Error"));
        return this;
    });
    bind(this, flush, ^{
        fflush(this->raw_file);
        return this;
    });
    bind(this, close, ^{
        fclose(this->raw_file);
    });
    bind(this, rewind, ^{
        rewind(this->raw_file);
        return this;
    });
    bind(this, seek, ^(long offset, int origin){
        fseek(this->raw_file, offset, origin);
        return this;
    });
    bind(this, tell, ^{
        return ftell(this->raw_file);
    });
    return 0;
};