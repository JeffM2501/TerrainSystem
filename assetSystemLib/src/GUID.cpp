#include "GUID.h"

namespace AssetSystem
{
    // Implementation

    const GUID GUID::Invalid(0,0);

    GUID::GUID() : m_data{ 0 } 
    {
    }

    GUID::GUID(const GUID& other) : m_data(other.m_data)
    {
    }

    GUID::GUID(const char* str) 
    {
        *this = Parse(str);
    }

    GUID::GUID(const std::string& str) 
    {
        *this = Parse(str);
    }

    GUID::GUID(const uint8_t* data) 
    {
        std::copy(data, data + 16, m_data.begin());
    }

    GUID::GUID(const std::vector<uint8_t>& data) 
    {
        std::copy(data.begin(), data.begin() + 16, m_data.begin());
    }

    GUID::GUID(const uint64_t& data1, const uint64_t& data2)
    {
        std::memcpy(m_data.data(), &data1, 8);
        std::memcpy(m_data.data() + 8, &data2, 8);
    }

    GUID& GUID::operator=(const GUID& other) {
        m_data = other.m_data;
        return *this;
    }

    GUID& GUID::operator=(const char* str)
    {
        *this = Parse(str);
        return *this;
    }

    GUID& GUID::operator=(const std::string& str) 
    {
        *this = Parse(str);
        return *this;
    }

    GUID& GUID::operator=(const uint8_t* data)
    {
        std::copy(data, data + 16, m_data.begin());
        return *this;
    }

    GUID& GUID::operator=(const std::vector<uint8_t>& data) 
    {
        std::copy(data.begin(), data.begin() + 16, m_data.begin());
        return *this;
    }

    bool GUID::operator==(const GUID& other) const {
        return m_data == other.m_data;
    }

    bool GUID::operator!=(const GUID& other) const 
    {
        return m_data != other.m_data;
    }

    bool GUID::operator<(const GUID& other) const
    {
        return m_data < other.m_data;
    }

    bool GUID::operator>(const GUID& other) const 
    {
        return m_data > other.m_data;
    }

    bool GUID::operator<=(const GUID& other) const 
    {
        return m_data <= other.m_data;
    }

    bool GUID::operator>=(const GUID& other) const 
    {
        return m_data >= other.m_data;
    }

    GUID::operator std::string() const 
    {
        return ToString();
    }

    std::string GUID::ToString() const 
    {
        std::ostringstream oss;
        for (size_t i = 0; i < m_data.size(); ++i) 
        {
            if (i == 4 || i == 6 || i == 8 || i == 10) 
            {
                oss << '-';
            }
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(m_data[i]);
        }
        return oss.str();
    }

    std::vector<uint8_t> GUID::ToData() const {
        return std::vector<uint8_t>(m_data.begin(), m_data.end());
    }

    GUID GUID::Generate() 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint16_t> dis(0, 255);

        GUID guid;
        for (auto& byte : guid.m_data) 
        {
            byte = uint8_t(dis(gen));
        }

        // Set the version to 4 (randomly generated UUID)
        guid.m_data[6] = (guid.m_data[6] & 0x0F) | 0x40;
        // Set the variant to 2 (RFC 4122)
        guid.m_data[8] = (guid.m_data[8] & 0x3F) | 0x80;

        return guid;
    }

    GUID GUID::Parse(const char* str) 
    {
        return Parse(std::string(str));
    }

    GUID GUID::Parse(const std::string& str)
    {
        GUID guid;
        size_t j = 0;
        for (size_t i = 0; i < str.size(); ++i) 
        {
            if (str[i] == '-') continue;
            guid.m_data[j++] = std::stoi(str.substr(i, 2), nullptr, 16);
            ++i;
        }
        return guid;
    }

    GUID GUID::Parse(const uint8_t* data) 
    {
        return GUID(data);
    }

    GUID GUID::Parse(const std::vector<uint8_t>& data) 
    {
        return GUID(data);
    }
}