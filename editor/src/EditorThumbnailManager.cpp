#include "EditorThumbnailManager.h"

#include "CRC64.h"
#include <unordered_map>

namespace Editor
{
    namespace ThumbnailManager
    {
        static std::unordered_map<size_t, Texture2D> ThumbnailCache;
        Texture2D GetThumbnail(std::string_view path, size_t* hash)
        {
            size_t crc = Hashes::CRC64Str(path);
            if (hash)
                *hash = crc;

            auto it = ThumbnailCache.find(crc);
            if (it != ThumbnailCache.end())
                return it->second;

            // Load the thumbnail from the file system
            Texture2D thumbnail = LoadTexture(path.data());
            ThumbnailCache[crc] = thumbnail;
            return thumbnail;
        }

        Texture2D GetThumbnail(size_t hash)
        {
            auto it = ThumbnailCache.find(hash);
            if (it != ThumbnailCache.end())
                return it->second;
            return {};
        }
    }
}