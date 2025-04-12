#pragma once

#include "raylib.h"
#include "Events.h"
#include "Toolbar.h"
#include "Camera.h"
#include "CRC64.h"
#include "Menu.h"

#include <functional>
#include <memory>
#include <string_view>
#include <string>

namespace EditorFramework
{
	class Document
	{
	public:
		virtual ~Document() = default;

		virtual uint64_t GetDocumentTypeID() = 0;
		void Created(size_t documentID) { DocumentID = documentID; OnCreated(); }

		inline size_t GetDocumentID() const { return DocumentID; }

		virtual std::string_view GetFileExtension() { return ""; }

		virtual void OnCreated() {};
		virtual void OnSetupMenu() {};

		virtual void OnActivated() {};
		virtual void OnDeactivated() {};

		virtual void OnUpdate(int width, int height) {};

		virtual void OnShowUI() {}
		virtual void OnShowContent(int width, int height) {}

		virtual std::string_view GetDocumentName() = 0;
		virtual bool GetDocumentDirty() { return false; }

		virtual bool UsesAsset() { return false; }

		virtual std::string_view GetAssetPath() const { return AssetPath; }
		virtual void CreateAsset() { AssetPath.clear(); SetDirty(); }
		virtual void OpenAsset(const std::string& assetPath) { AssetPath = assetPath; }
		virtual void SaveAsset() { SetClean(); };
		virtual void SaveAsAsset(const std::string& assetPath) { AssetPath = assetPath; }

		virtual bool IsDirty() const { return Dirty; }

		virtual void SetClean() { Dirty = false; }

		virtual Toolbar* GetToolbar() { return &MainToolbar; }
		virtual MenuBar& GetDocumentMenu() { return DocumentMenuBar; }
        virtual CommandContextSet* GetDocumentCommandContext() { return &DocumentCommandContext; }

		Events::EventSource<Document> OnDirty;

        virtual void SetDirty()
        {
            Dirty = true;
            OnDirty.Invoke(*this);
        }

	protected:
		size_t DocumentID = 0;
		std::string AssetPath;

		Toolbar MainToolbar;
		MenuBar DocumentMenuBar;

		CommandContextSet DocumentCommandContext;

		bool Dirty = false;

		class Application* GetApp() { return HostApp; }

	private:
		friend class Application;
		class Application* HostApp = nullptr;
	};

	using DocumentPtr = std::unique_ptr<Document>;
	using DocumentFactory = std::function<DocumentPtr()>;

	static constexpr char NO_EXTENSION[] = "";

	class ViewportDocument : public Document
	{
	public:
		ViewportDocument();
		void OnShowContent(int width, int height) override;
		void OnUpdate(int width, int height) override;

        EditorCamera& GetCamera() { return VieportCamera; }

	protected:
		virtual void OnShowScene(const Vector2& renderSize) = 0;

	protected:
		EditorCamera VieportCamera;

		Color ClearColor = SKYBLUE;
		bool ZIsUp = true;
	};
}

#define REGISTER_DOCUMENT(T, E) \
	static EditorFramework::DocumentPtr Factory() { return std::make_unique<T>(); } \
	static uint64_t DocumentTypeID() { return Hashes::CRC64Str(#T); } \
	static std::string_view FileExtension() { return #E; } \
	static std::string_view GetDocumentTypeName() { return #T; } \
	inline uint64_t GetDocumentTypeID() override { return T::DocumentTypeID(); } \
	inline std::string_view GetFileExtension()override { return #E; }