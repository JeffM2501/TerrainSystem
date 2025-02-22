#pragma once

#include "raylib.h"
#include <deque>
#include <string>
#include <memory>
#include <functional>

#include "imgui.h"

namespace EditorFramework
{
    static constexpr char DialogOK[] = "OK";
    static constexpr char DialogCancel[] = "Cancel";
    static constexpr char DialogYes[] = "Yes";
    static constexpr char DialogNo[] = "No";
    static constexpr char DialogApply[] = "Apply";

    ImVec2 GetButtonSize(std::string_view label);

    enum class DialogResult
    {
        None,
        Accept,
        Decline,
        Cancel
    };

    class Dialog;
    using DialogShowCallback = std::function<DialogResult(Dialog&)>;
    using DialogResultCallback = std::function<void(Dialog&, DialogResult)>;
    using DialogCallback = std::function<void(Dialog&)>;

    class Dialog
    {
    public:
        virtual ~Dialog() = default;

        bool Process();

        DialogShowCallback          ShowCallback;
        DialogResultCallback        ResultsCallback;
        DialogCallback              AcceptCallback;
        DialogCallback              DeclineCallback;
        DialogCallback              CancelCallback;

    protected:
        virtual void OnCreate() {}
        virtual DialogResult OnShow() = 0;
        virtual void OnUpdate() {};

        virtual bool EnableAccept() { return true; }
        
        void Create();

        virtual void OnResults(DialogResult results);

        virtual void OnAccept();
        virtual void OnDecline();
        virtual void OnCancel();

		template <class T, typename ...Args>
		void ShowDialogBox(Args&&... args)
		{
			auto dlog = std::make_unique<T>(std::forward<Args>(args)...);
            ChildDialogs.emplace_back(std::move(dlog));
		}

    protected:
        std::string AcceptName;
        std::string DeclineName;
        std::string CancelName;

        std::string DialogName;

        ImVec2 MinimumSize = { 200,100 };
        bool Resizeable = false;

        std::deque<std::unique_ptr<Dialog>> ChildDialogs;

    private:
        bool Created = false;

        std::string ImGuiName;
    };

    class MessageBox : public Dialog
    {
    private:
        std::string Message;

    protected:
        DialogResult OnShow() override
        {
            ImGui::TextUnformatted(Message.c_str());

            return DialogResult::None;
        }

    public:
        MessageBox(std::string_view message, std::string_view title = "", DialogCallback onAccept = nullptr)
            : Message(message)
        {
            DialogName = title;
            if (DialogName.empty())
                DialogName = "Message";

            AcceptName = DialogOK;
            AcceptCallback = onAccept;
        }
    };

    class SaveDirtyFileDialog : public Dialog
    {
    private:
        std::string Filename;

    protected:
        DialogResult OnShow() override;
    public:
        SaveDirtyFileDialog(std::string_view file, DialogCallback onAccept, DialogCallback onDecline = nullptr, DialogCallback onCancel = nullptr);
    };
}