#pragma once

#include "AssetManager.h"

#include "rapidjson/document.h"

#include <functional>
#include <memory>

namespace AssetSystem
{
    using AssetReader = std::function<void(Asset::Ptr asset, const rapidjson::Document& document)>;
    using AssetWriter = std::function<void(const Asset::Ptr asset, rapidjson::Document& document)>;

    namespace AssetIO
    {

    }
}