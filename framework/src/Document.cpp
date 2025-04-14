#include "Document.h"

#include "rlgl.h"
#include "raylib.h"

namespace EditorFramework
{
	ViewportDocument::ViewportDocument()
	{
		VieportCamera.GetCamera()->fovy = 45;
		if (ZIsUp)
		{
			VieportCamera.GetCamera()->up.z = 1;
			VieportCamera.GetCamera()->position.y = -10;
		}
		else
		{
			VieportCamera.GetCamera()->up.y = 1;
			VieportCamera.GetCamera()->position.z = -10;
		}

		VieportCamera.PushController<OribitCameraController>();
		VieportCamera.PushController<FPSCameraController>();
		VieportCamera.PushController<FocusCameraController>();
	}

	void ViewportDocument::OnShowContent(int width, int height)
	{
		ClearBackground(ClearColor);
		rlSetClipPlanes(NearPlane, FarPlane);
		VieportCamera.Apply();

		OnShowScene(Vector2{ float(width), float(height) });

		if (VieportCamera.IsConrollerActive<OribitCameraController>())
		{
			rlDrawRenderBatchActive();
			rlDisableDepthTest();
			auto target = VieportCamera.GetCamera()->target;
			rlPushMatrix();
			rlTranslatef(target.x, target.y, target.z);
			constexpr float size = 0.5f;
			DrawLine3D(Vector3{ -size,0,0 }, Vector3{ size ,0,0 }, RED);
			DrawLine3D(Vector3{ 0,-size,0 }, Vector3{ 0, size,0 }, GREEN);
			DrawLine3D(Vector3{ 0,0,-size }, Vector3{ 0,0,size }, BLUE);
			rlPopMatrix();
			rlDrawRenderBatchActive();
			rlEnableDepthTest();
		}
		EndMode3D();

		rlSetClipPlanes(RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
	}

	void ViewportDocument::OnUpdate(int width, int height)
	{
		VieportCamera.Update(Vector2{ float(width), float(height) });
	}
}