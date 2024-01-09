#include <iostream>
#include "JPEGdecoder.hpp"
#include "encoding.h"

int main()
{   
    std::string inpname;
    std::cout << "Enter input jpg name: ";
    std::cin >> inpname;
    DecodeJPEG(inpname);
    auto enc = JPGencoder();
    enc.start();
}