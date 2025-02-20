#pragma once

#include "raylib.h"
#include "Events.h"
#include "Toolbar.h"

#include <functional>
#include <memory>
#include <string_view>
#include <string>

namespace EditorFramework
{
	class Document
	{
	public:
		virtual size_t GetDocumentTypeID() = 0;
		void Created(size_t documentID) { DocumentID = documentID; OnCreated(); }

		inline size_t GetDocumentID() const { return DocumentID; }

		virtual std::string_view GetFileExtension() { return ""; }

		virtual void OnCreated() {};
		virtual void OnActivated() {};
		virtual void OnDeactivated() {};

		virtual void OnUpdate(int width, int height) {};

		virtual void OnShowUI() {}
		virtual void OnShowContent(int width, int height) {}

		virtual std::string_view GetDocumentName() = 0;
		virtual bool GetDocumentDirty() { return false; }

		virtual bool UsesAsset() { return false; }

		virtual std::string_view GetAssetPath() const { return AssetPath; }
		virtual void OpenAsset(std::string_view assetPath) { AssetPath = assetPath; }
		virtual void SaveAsset() { SetClean(); };
		virtual void SaveAsAsset(std::string_view assetPath) { AssetPath = assetPath; }

		virtual bool IsDirty() const { return Dirty; }

		virtual void SetClean() { Dirty = false; }

		virtual Toolbar* GetToolbar() { return &MainToolbar; }

		Events::EventSource<Document> OnDirty;

	protected:
		virtual void SetDirty()
		{
			Dirty = true; 
			OnDirty.Invoke(*this); 
		}

		size_t DocumentID = 0;
		std::string AssetPath;

		Toolbar MainToolbar;

		bool Dirty = false;

		class Application* GetApp() { return HostApp; }

	private:
		friend class Application;
		class Application* HostApp = nullptr;
	};

	using DocumentPtr = std::unique_ptr<Document>;
	using DocumentFactory = std::function<DocumentPtr()>;

	static constexpr char NO_EXTENSION[] = "";
}

#define REGISTER_DOCUMENT(T, E) \
	static EditorFramework::DocumentPtr Factory() { return std::make_unique<T>(); } \
	static size_t DocumentTypeID() { static std::hash<std::string_view> hasher; return hasher(#T); } \
	static std::string_view FileExtension() { return #E; } \
	static std::string_view GetDocumentTypeName() { return #T; } \
	inline size_t GetDocumentTypeID() override { return T::DocumentTypeID(); } \
	inline std::string_view GetFileExtension()override { return #E; }
