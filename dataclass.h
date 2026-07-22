#include "oop.h"
#include "oop.h"

class (Integer)
{
    int int_value;
};

ctor(Integer, int value)
{
    this->int_value = value;
    return 0;
};

dtor(Integer) {};

class (Float)
{
    int float_value;
};

ctor(Float, float value)
{
    this->float_value = value;
    return 0;
};

dtor(Float) {};

class (Character)
{
    char char_value;
};

ctor(Character, char value)
{
    this->char_value = value;
    return 0;
};