#include <iostream>
#include "JPEGdecoder.hpp"
#include "encoding.h"

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
    DecodeJPEG(argv[1]);
    auto enc = JPGencoder();
    enc.start(argv[2]);
}