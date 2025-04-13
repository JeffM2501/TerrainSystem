#pragma once

#include "raylib.h"
#include <string>

namespace Editor
{
    namespace ThumbnailManager
    {
        Texture2D GetThumbnail(std::string_view path, size_t* hash = nullptr);
        Texture2D GetThumbnail(size_t hash);
    }
}