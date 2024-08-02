#include <iostream>
#include "Logger.h"

int main(int argc, char* argv[])
{
    LOG(Debug, "Hello There !");

    std::cin.get();
    
    return 0;
}
