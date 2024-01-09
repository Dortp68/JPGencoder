#include <iostream>
#include "JPEGcoder.hpp"
#include "decoding.h"

int main()
{
    auto dec = JPGdecoder();
    dec.start();
}