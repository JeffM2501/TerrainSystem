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
			VieportCamera.GetCamera()->target.y = 10;
		}
		else
		{
			VieportCamera.GetCamera()->up.y = 1;
			VieportCamera.GetCamera()->target.z = 10;
		}

		VieportCamera.PushController<OribitCameraController>();
		VieportCamera.PushController<FPSCameraController>();
		VieportCamera.PushController<AnimateViewController>();
	}

	void ViewportDocument::OnShowContent(int width, int height)
	{
		ClearBackground(ClearColor);
		VieportCamera.Apply();

		OnShowScene(Vector2{ float(width), float(height) });

		if (VieportCamera.IsConrollerActive<OribitCameraController>())
		{
			rlDisableDepthTest();
			DrawSphere(VieportCamera.GetCamera()->target, 0.25f, RED);
			rlDrawRenderBatchActive();
			rlEnableDepthTest();
		}
		EndMode3D();
	}

	void ViewportDocument::OnUpdate(int width, int height)
	{
		VieportCamera.Update(Vector2{ float(width), float(height) });
	}
}