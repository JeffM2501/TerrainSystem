#pragma once

#include <vector>

namespace Types
{
    class FieldPath
    {
    public:
        enum class ElementType
        {
            Unknown,
            Field,
            Index
        };

        struct Element
        {
            ElementType Type = ElementType::Unknown;
            int Index = 0;
        };

        static Element Field(int index) { return Element{ ElementType::Field, index }; }
        static Element Index(int index) { return Element{ ElementType::Index, index }; }

        std::vector<Element> Elements;


        void PushFront(const FieldPath& path)
        {
            Elements.insert(Elements.begin(), path.Elements.begin(), path.Elements.end());
        }

        void PushBack(const FieldPath& path)
        {
            Elements.insert(Elements.end(), path.Elements.begin(), path.Elements.end());
        }

        FieldPath() = default;

        FieldPath(const FieldPath& path) { Elements = path.Elements; }
        FieldPath(const FieldPath& path, const Element& element) noexcept { Elements = path.Elements; Elements.push_back(element); }
        FieldPath(const Element& element) { Elements.push_back(element); }
        FieldPath(ElementType elementType, int index) { Elements.push_back(Element{ elementType, index }); }


        FieldPath operator+ (const FieldPath& rhs) const
        {
            FieldPath path(*this);
            path.PushBack(rhs);
            return path;
        }

        FieldPath operator+ (const FieldPath::Element& rhs) const
        {
            FieldPath path(*this);
            path.Elements.push_back(rhs);
            return path;
        }

        FieldPath& operator+= (const FieldPath& rhs)
        {
            PushBack(rhs);
            return *this;
        }

        FieldPath& operator+= (const FieldPath::Element& rhs)
        {
            Elements.push_back(rhs);
            return *this;
        }

        bool operator== (const FieldPath& rhs) const
        {
            if (Elements.size() != rhs.Elements.size())
                return false;
            for (size_t i = 0; i < Elements.size(); i++)
            {
                if (Elements[i].Type != rhs.Elements[i].Type || Elements[i].Index != rhs.Elements[i].Index)
                    return false;
            }
            return true;
        }

        bool operator!= (const FieldPath& rhs) const
        {
            return !(*this == rhs);
        }

        static FieldPath GetCommonAncestorPath(const FieldPath& path1, const FieldPath& path2)
        {
            FieldPath commonPath;
            size_t minSize = std::min(path1.Elements.size(), path2.Elements.size());
            for (size_t i = 0; i < minSize; i++)
            {
                if (path1.Elements[i].Type == path2.Elements[i].Type && path1.Elements[i].Index == path2.Elements[i].Index)
                    commonPath.Elements.push_back(path1.Elements[i]);
                else
                    break;
            }
            return commonPath;
        }

    };
}