#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include "raylib.h"

namespace EditorFramework
{
	class EditorCamera;

#define DEFINE_CAMERA_CONTROLLER(T) \
	static size_t ControlllerTypeID() { static std::hash<std::string_view> hasher; return hasher(#T); } \
	inline size_t GetControlllerTypeID() override { return T::ControlllerTypeID(); } 

	class EditorCameraController
	{
	public:
		~EditorCameraController() = default;

		virtual bool Update(EditorCamera& camera, const Vector2& renderSize) = 0;

		virtual size_t GetControlllerTypeID() = 0;

		float TranslateSpeed = 100.0f;
		float FastTranslateModifyer = 10.0f;
        float MouseSensitivity = 1.0f / 500.0f;

	protected:
        void ApplyTranslation(EditorCamera& camera);
	};

    class FocusCameraController : public EditorCameraController
    {
    public:
        DEFINE_CAMERA_CONTROLLER(FocusCameraController);
        bool Update(EditorCamera& camera, const Vector2& renderSize) final;

		void SetFocusPoint(EditorCamera& camera, const Vector3& point, float time = 1.0f, float distance = 10.0f, bool focusFirst = true);

    private:
        Vector3 DesiredCameraPosition = { 0, 0, 0 };
        Vector3 DesiredCameraTarget = { 0, 0, 0 };
		float TotalTime = 0;
        float CurrentTime = 0;
    };

	class FPSCameraController : public EditorCameraController
	{
	public:
		DEFINE_CAMERA_CONTROLLER(FPSCameraController);
		bool Update(EditorCamera& camera, const Vector2& renderSize) final;
	};

	class OribitCameraController : public EditorCameraController
	{
	public:	
        DEFINE_CAMERA_CONTROLLER(OribitCameraController);
		bool Update(EditorCamera& camera, const Vector2& renderSize) final;

	private:
		float MouseWheelSenstivity = 1.0f / 2.0f;
	};

	class EditorCamera
	{
		public:
			std::vector<std::unique_ptr<EditorCameraController>> Controllers;

			template<class T, typename ...Args>
			void PushController(Args&&... args)
			{
				auto controller = std::make_unique<T>(std::forward<Args>(args)...);
				Controllers.emplace_back(std::move(controller));
			}

            EditorCameraController* FindController(size_t controllerID)
            {
                for (auto& controller : Controllers)
                {
                    if (controller->GetControlllerTypeID() == controllerID)
                        return controller.get();
                }
                return nullptr;
            }
            EditorCameraController* FindController(std::string_view controllerName)
            {
                for (auto& controller : Controllers)
                {
                    if (controller->GetControlllerTypeID() == std::hash<std::string_view>{}(controllerName))
                        return controller.get();
                }
                return nullptr;
            }
			template <class T>
            T* FindController()
            {
                return static_cast<T*>(FindController(T::ControlllerTypeID()));
            }

			void Update(const Vector2& renderSize);
			void Apply();

			void SetFOV(float fovy) { ViewCamera.fovy = fovy; }
			float GetFOVY() const { return ViewCamera.fovy; }

			Camera3D* GetCamera() { return &ViewCamera; }

            EditorCameraController* GetActiveController() { return ActiveController; }

			template <class T>
            bool IsConrollerActive()
            {
				if (!ActiveController)
					return false;

                return ActiveController->GetControlllerTypeID() == T::ControlllerTypeID();
            }

		private:
			Camera3D	ViewCamera = { 0 };
            EditorCameraController* ActiveController = nullptr;
	};
}