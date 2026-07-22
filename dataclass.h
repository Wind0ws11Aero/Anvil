#include "oop.h"
#include "string.h"
#include "utils.h"

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
    float float_value;
};

ctor(Float, float value)
{
    this->float_value = value;
    return 0;
};

dtor(Float) {};

class (Double)
{
    double double_value;
};

ctor(Double, float value)
{
    this->double_value = value;
    return 0;
};

dtor(Double) {};

class (Character)
{
    char char_value;
};

ctor(Character, char value)
{
    this->char_value = value;
    return 0;
};

dtor(Character) {};

#define __to_dataclass_one(x)                                                                       \
    _Generic((x),                                                                                   \
        int: new(Integer, x),                                                                       \
        float: new(Float, x),                                                                       \
        double: new(Double, x),                                                                     \
        char: new(Character, x), char *: new(String, x))

#define to_dataclass(...) EXPAND_ALL(__to_dataclass_one, __VA_ARGS__)
