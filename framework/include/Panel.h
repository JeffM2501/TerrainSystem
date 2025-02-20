#pragma once

#include "raylib.h"

#include "imgui.h"

#include <string>
#include <string_view>

namespace EditorFramework
{
    class Document;

    enum class PlanelLayoutLocation : uint8_t
    {
        Floating,
        Left,
        Right,
        Bottom,
        Max
    };

    class Panel
    {
    public:
        void Update();

        void Open() { Shown = true; }
        void Close() { Shown = false; }

        bool IsOpen() { return Shown; }

        void ForceDockSpace(ImGuiID id) { ForcedDockingID = id; }

        inline void DocumentChanged(Document* document)
        {
            OnDocumentChanged(document);
            CurrentDocument = document;
        }

        std::string_view GetImGuiName();

        std::string_view GetName() const { return Name; }
        std::string_view GetIcon() const { return Icon; }

        PlanelLayoutLocation Location = PlanelLayoutLocation::Floating;

    protected:
        virtual void OnUpdate() {};
        virtual void OnShow() {};
        virtual void OnDocumentChanged(Document*) {};

        std::string Icon;
        std::string Name;

        Document* GetDocument() { return CurrentDocument; }

        template<class T>
        T* GetDoumentAs()
        {
            if (!CurrentDocument || CurrentDocument->GetDocumentTypeID() != T::DocumentTypeID())
                return nullptr;

            return static_cast<T*>(CurrentDocument);
        }
    private:
        bool Shown = true;

        ImGuiID ForcedDockingID = 0;

        Document* CurrentDocument;

        Vector2 MiniumSize = { 300, 300 };

        std::string ImGuiName;
    };
}