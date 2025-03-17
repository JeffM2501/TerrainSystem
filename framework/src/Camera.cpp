#include "Camera.h"

#include "Action.h"
#include "StandardActions.h"

#include "rcamera.h"
#include "raymath.h"

namespace EditorFramework
{
	void EditorCamera::Update(const Vector2& renderSize)
	{
		for (auto& controller : Controllers)
		{
			if (controller->Update(*this, renderSize))
			{
                ActiveController = controller.get();
				return;
			}
		}
		ActiveController = nullptr;
	}

	void EditorCamera::Apply()
	{
		BeginMode3D(ViewCamera);
	}

    void EditorCameraController::ApplyTranslation(EditorCamera& camera)
    {
        float translateDistance = TranslateSpeed * GetFrameTime();

        if (ActionRegistry::IsActionHeld(CameraFastMoveAction))
            translateDistance *= FastTranslateModifyer;

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
    }

	bool FPSCameraController::Update(EditorCamera& camera, const Vector2& renderSize)
	{
		if (!ActionRegistry::IsActionHeld(FPSModeAction, true))
			return false;

        ApplyTranslation(camera);

		CameraPitch(camera.GetCamera(), GetMouseDelta().y * -MouseSensitivity, true, false, false);
		CameraYaw(camera.GetCamera(), GetMouseDelta().x * -MouseSensitivity, false);
		return true;
	}

	bool OribitCameraController::Update(EditorCamera& camera, const Vector2& renderSize)
	{
		if (!ActionRegistry::IsActionHeld(OrbitModeAction, true) || !ActionRegistry::IsActionHeld(FPSModeAction, true))
			return false;

        ApplyTranslation(camera);

        CameraMoveToTarget(camera.GetCamera(), GetMouseWheelMove() * -MouseWheelSenstivity);

        CameraPitch(camera.GetCamera(), GetMouseDelta().y * -MouseSensitivity, true, true, false);
        CameraYaw(camera.GetCamera(), GetMouseDelta().x * -MouseSensitivity, true);

		return true;
	}

    bool FocusCameraController::Update(EditorCamera& camera, const Vector2& renderSize)
    {
		if (CurrentTime >= TotalTime)
			return false;

        CurrentTime += GetFrameTime();
		if (CurrentTime > TotalTime)
			CurrentTime = TotalTime;

		float param = CurrentTime / TotalTime;

        camera.GetCamera()->position = Vector3Lerp(StartCameraPosition, DesiredCameraPosition, param);
        camera.GetCamera()->target = Vector3Lerp(StartCameraTarget, DesiredCameraTarget, param);

		return true;
    }

    void FocusCameraController::SetFocusPoint(EditorCamera& camera, const Vector3& point, float time /*= 1.0f*/, float distance /*= 10.0f*/, bool focusFirst /*= true*/)
    {
		//Vector3 forward = Vector3Normalize(GetCameraForward(camera.GetCamera()));
       
		StartCameraPosition = camera.GetCamera()->position;
        StartCameraTarget = camera.GetCamera()->target;
        if (focusFirst)
            StartCameraTarget = DesiredCameraPosition;

		Vector3 forward = Vector3Normalize(StartCameraTarget - StartCameraPosition);

        DesiredCameraPosition = point + Vector3Scale(forward, -distance);
		DesiredCameraTarget = point;
	
		CurrentTime = 0;
		TotalTime = time;
    }
}