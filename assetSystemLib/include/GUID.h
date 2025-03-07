#pragma once

#include <string>
#include <vector>
#include <array>
#include <random>
#include <sstream>
#include <iomanip>

namespace AssetSystem
{
    class GUID
    {
    public:
        GUID();
        GUID(const GUID& other);
        GUID(const char* str);
        GUID(const std::string& str);
        GUID(const uint8_t* data);
        GUID(const std::vector<uint8_t>& data);
        GUID(const uint64_t& data1, const uint64_t& data2);
        ~GUID() = default;

        GUID& operator=(const GUID& other);
        GUID& operator=(const char* str);
        GUID& operator=(const std::string& str);
        GUID& operator=(const uint8_t* data);
        GUID& operator=(const std::vector<uint8_t>& data);

        bool operator==(const GUID& other) const;
        bool operator!=(const GUID& other) const;
        bool operator<(const GUID& other) const;
        bool operator>(const GUID& other) const;
        bool operator<=(const GUID& other) const;
        bool operator>=(const GUID& other) const;

        operator std::string() const;

        std::string ToString() const;
        std::vector<uint8_t> ToData() const;

        static GUID Generate();
        static GUID Parse(const char* str);
        static GUID Parse(const std::string& str);
        static GUID Parse(const uint8_t* data);
        static GUID Parse(const std::vector<uint8_t>& data);

        static const GUID Invalid;

    private:
        std::array<uint8_t, 16> m_data;
    };

}

namespace std
{
    template<>
    struct hash<AssetSystem::GUID>
    {
        size_t operator()(const AssetSystem::GUID& guid) const
        {
            std::hash<std::string> hasher;
            return hasher(guid.ToString());
        }
    };
}