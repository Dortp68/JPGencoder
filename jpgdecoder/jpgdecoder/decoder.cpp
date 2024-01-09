#include <iostream>
#include "JPEGcoder.hpp"
#include "decoding.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Source or output file is not provided!" << std::endl;
        return -1;
    }
    else if (argc > 3)
    {
        std::cout << "Too many arguments!" << std::endl;
        return -1;
    }
    auto dec = JPGdecoder();
    dec.start(argv[1], argv[2]);
    return 0;
}