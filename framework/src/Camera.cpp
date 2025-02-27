#include "Camera.h"

#include "Action.h"
#include "StandardActions.h"

#include "rcamera.h"

namespace EditorFramework
{
	void EditorCamera::Update(const Vector2& renderSize)
	{
		for (auto& controller : Controllers)
		{
			if (controller->Update(*this, renderSize))
				break;
		}
	}

	void EditorCamera::Apply()
	{
		BeginMode3D(ViewCamera);
	}

	bool FPSCameraController::Update(EditorCamera& camera, const Vector2& renderSize)
	{
		if (!ActionRegistry::IsActionHeld(FPSModeAction, true))
			return false;

		float translateDistance = 100 * GetFrameTime();

		if (ActionRegistry::IsActionHeld(CameraFastMoveAction))
			translateDistance *= 10;

		if (ActionRegistry::IsActionHeld(CameraTranslateFowardAction, true))
			CameraMoveForward(camera.GetCamera(), translateDistance, false);

		if (ActionRegistry::IsActionHeld(CameraTranslateBackwardsAction, true))
			CameraMoveForward(camera.GetCamera(), -translateDistance, false);

		if (ActionRegistry::IsActionHeld(CameraTranslateLeftAction, true))
			CameraMoveRight(camera.GetCamera(), -translateDistance, false);

		if (ActionRegistry::IsActionHeld(CameraTranslateRightAction, true))
			CameraMoveRight(camera.GetCamera(), translateDistance, false);

		if (ActionRegistry::IsActionHeld(CameraTranslateUpAction, true))
			CameraMoveUp(camera.GetCamera(), translateDistance);

		if (ActionRegistry::IsActionHeld(CameraTranslateDownAction, true))
			CameraMoveUp(camera.GetCamera(), -translateDistance);

		float mouseSensitivity = 1.0f/500.0f;

		CameraPitch(camera.GetCamera(), GetMouseDelta().y * -mouseSensitivity, true, false, false);
		CameraYaw(camera.GetCamera(), GetMouseDelta().x *- mouseSensitivity, false);
		return true;
	}

	bool OribitCameraController::Update(EditorCamera& camera, const Vector2& renderSize)
	{
		if (!ActionRegistry::IsActionHeld(OrbitModeAction))
			return false;

		return true;
	}

}