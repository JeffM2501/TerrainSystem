#pragma once

#include <memory>
#include <vector>

#include "raylib.h"

namespace EditorFramework
{
	class EditorCamera;

	class EditorCameraController
	{
	public:
		~EditorCameraController() = default;

		virtual bool Update(EditorCamera& camera, const Vector2& renderSize) = 0;
	};

	class FPSCameraController : public EditorCameraController
	{
	public:
		bool Update(EditorCamera& camera, const Vector2& renderSize) final;
	};

	class OribitCameraController : public EditorCameraController
	{
	public:
		bool Update(EditorCamera& camera, const Vector2& renderSize) final;

	private:
		Vector3	OrbitPos;
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

			void Update(const Vector2& renderSize);
			void Apply();

			void SetFOV(float fovy) { ViewCamera.fovy = fovy; }
			float GetFOVY() const { return ViewCamera.fovy; }

			Camera3D* GetCamera() { return &ViewCamera; }

		private:
			Camera3D	ViewCamera = { 0 };
	};
}