#pragma once
#include <cinttypes>
#include <string>

namespace Hashes
{
    uint64_t CRC64(const unsigned char* s, uint64_t l);

    inline uint64_t CRC64Str(std::string_view data)
    {
        return CRC64((const unsigned char*)(data.data()), data.size());
    }
}
