#include "Document.h"

namespace EditorFramework
{
	ViewportDocument::ViewportDocument()
	{
		VieportCamera.GetCamera()->fovy = 45;
		if (ZIsUp)
		{
			VieportCamera.GetCamera()->up.z = 1;
			VieportCamera.GetCamera()->target.y = 1;
		}
		else
		{
			VieportCamera.GetCamera()->up.y = 1;
			VieportCamera.GetCamera()->target.z = 1;
		}


		VieportCamera.PushController<OribitCameraController>();
		VieportCamera.PushController<FPSCameraController>();
	}

	void ViewportDocument::OnShowContent(int width, int height)
	{
		ClearBackground(ClearColor);
		VieportCamera.Apply();

		OnShowScene(Vector2{ float(width), float(height) });
		EndMode3D();
	}

	void ViewportDocument::OnUpdate(int width, int height)
	{
		VieportCamera.Update(Vector2{ float(width), float(height) });
	}
}