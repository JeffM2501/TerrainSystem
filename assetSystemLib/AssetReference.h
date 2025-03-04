#pragma once

#include "GUID.h"

namespace AssetSystem
{
    template<class T>
    struct AssetReference
    {
    public:
        GUID AssetId;

    private:
        uint64_t AssetTypeID = 0;
    };

    struct ResrouceReference
    {
        uint64_t ResourceHash;
        std::string ResourcePath;
    };
}
