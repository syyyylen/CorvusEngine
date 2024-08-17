#pragma once
#include "Core.h"

struct Image
{
    ~Image();

    void LoadImageFromFile(const std::string& path, bool flip = true);
    
    char* Bytes;
    int Width;
    int Height;
};