#include "Image.h"
#include <stb/stb_image.h>

Image::~Image()
{
    if(Bytes != nullptr)
    {
        delete[] Bytes;
        Bytes = nullptr;
    }
}

void Image::LoadImageFromFile(const std::string& path, bool flip)
{
    int channels;

    stbi_set_flip_vertically_on_load(flip);
    Bytes = reinterpret_cast<char*>(stbi_load(path.c_str(), &Width, &Height, &channels, STBI_rgb_alpha));
    if (!Bytes)
    {
        LOG(Error, "Image : Failed to load image " + path);
        return;        
    }

    LOG(Debug, "Image : Loaded image " + path);
}
