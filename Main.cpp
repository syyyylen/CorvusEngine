#include <iostream>

#include "CorvusEditor.h"
#include "Logger.h"

int main(int argc, char* argv[])
{
    LOG(Debug, "Hello There !");

    {
        CorvusEditor Editor;
        Editor.Run();
    }

    return 0;
}
