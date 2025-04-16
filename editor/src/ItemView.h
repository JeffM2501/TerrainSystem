#pragma once

#include <string>
#include "raylib.h"
#include "imgui.h"

class ViewableItem
{
public:
    virtual ~ViewableItem() = default;

    std::string Name;
    std::string Icon;
    ImVec4 Tint = { 1, 1, 1, 1 };
    bool Shown = true;
    bool Filtered = false;

    virtual const char* GetToolTip() { return Name.c_str(); }
};

class ViewableItemContainer
{
public:
    virtual ~ViewableItemContainer() = default;
    virtual ViewableItem* Reset() = 0;
    virtual size_t Count() = 0;
    virtual ViewableItem* Next() = 0;
};

class ItemView
{
public:
    virtual ~ItemView() = default;
    virtual ViewableItem* Show(ViewableItemContainer& container) = 0;
};

class ListItemView : public ItemView
{
public:
    ViewableItem* Show(ViewableItemContainer& container) override;
};

class GridItemView : public ItemView
{
private:
    float DesiredItemWidith = 80;

    bool ShowItem(ViewableItem* item);

public:
    GridItemView(float width)
        : DesiredItemWidith(width) {
    }

    ViewableItem* Show(ViewableItemContainer& container) override;
};