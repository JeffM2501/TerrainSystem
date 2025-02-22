#include "TerrainDocument.h"
#include "Application.h"
#include "DisplayScale.h"
#include "Dialog.h"

#include "extras/IconsFontAwesome6.h"

#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"
#include "rlgl.h"

static constexpr float CAMERA_MOVE_SPEED = 20;
static constexpr float CAMERA_ROTATION_SPEED = 1;
static constexpr float CAMERA_PAN_SPEED = 20;
static constexpr float CAMERA_ORBITAL_SPEED = 0.25f;
static constexpr float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.003f;

using namespace EditorFramework;

void UpdateCameraXY(Camera* camera, int mode)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = false;// ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FREE) || (mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = false;

    // Camera speeds based on frame time
    float cameraMoveSpeed = CAMERA_MOVE_SPEED * GetFrameTime();
    if (IsKeyDown(KEY_LEFT_SHIFT))
        cameraMoveSpeed *= 5;

    float cameraRotationSpeed = CAMERA_ROTATION_SPEED * GetFrameTime();
    float cameraPanSpeed = CAMERA_PAN_SPEED * GetFrameTime();
    float cameraOrbitalSpeed = CAMERA_ORBITAL_SPEED * GetFrameTime();

    if (mode == CAMERA_CUSTOM) {}
    else if (mode == CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(GetCameraUp(camera), cameraOrbitalSpeed);
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    }
    else
    {
        // Camera rotation
        if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) CameraPitch(camera, cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -cameraRotationSpeed, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, cameraRotationSpeed, rotateAroundTarget);


        // Camera movement
        // Camera pan (for CAMERA_FREE)
        if ((mode == CAMERA_FREE) && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)))
        {
            const Vector2 mouseDelta = GetMouseDelta();
            if (mouseDelta.x > 0.0f) CameraMoveRight(camera, cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.x < 0.0f) CameraMoveRight(camera, -cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.y > 0.0f) CameraMoveUp(camera, -cameraPanSpeed);
            if (mouseDelta.y < 0.0f) CameraMoveUp(camera, cameraPanSpeed);
        }
        else
        {
            // Mouse support
            CameraYaw(camera, -mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);
        }

        // Keyboard support
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_D)) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);

        if (IsKeyDown(KEY_Q)) CameraMoveUp(camera, -cameraMoveSpeed);
        if (IsKeyDown(KEY_E)) CameraMoveUp(camera, cameraMoveSpeed);

        // Gamepad movement
        if (IsGamepadAvailable(0))
        {
            // Gamepad controller support
            CameraYaw(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * 2) * CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * 2) * CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);
        }

        if (mode == CAMERA_FREE)
        {
            if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, cameraMoveSpeed);
            if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -cameraMoveSpeed);
        }
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL) || (mode == CAMERA_FREE))
    {
        // Zoom target distance
        CameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT)) CameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD)) CameraMoveToTarget(camera, -2.0f);
    }
}

std::string_view TerrainDocument::GetDocumentName()
{
    if (AssetPath.empty())
        return TextFormat("Untitled Terrain %d", DocumentID);
    else
        return GetFileNameWithoutExt(AssetPath.c_str());
}

void TerrainDocument::OnUpdate(int width, int height)
{
    if (GetApp()->MouseIsInDocument() && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        UpdateCameraXY(&Camera, CAMERA_THIRD_PERSON);
}

void TerrainDocument::OnShowContent(int width, int height)
{
    ClearBackground(DARKBLUE);

    BeginMode3D(Camera);
    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    DrawGrid(1024, 16);
    rlPopMatrix();

    DrawCube(Vector3Zeros, 1, 1, 1, RED);
    DrawCube(Vector3{ Info.TerrainGridSize * 1.0f, Info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
    DrawCube(Vector3{ 0, Info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
    DrawCube(Vector3{ Info.TerrainGridSize * 1.0f,0,0 }, 1, 1, 1, BLUE);

    DrawCube(Vector3{ 0,Info.TerrainGridSize * 0.5f,0 }, 0.5f, 0.5f, 0.5f, YELLOW);
    DrawCube(Vector3{ Info.TerrainGridSize * 0.5f,0,0 }, 0.5f, 0.5f, 0.5f, YELLOW);

    DrawCube(Vector3{ 0,1,0 }, 0.125f, 2, 0.125f, PURPLE);

    // draw terrain

    EndMode3D();
}

void TerrainDocument::OnShowUI()
{
    auto size = GetButtonSize(ICON_FA_BOX);

    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
    ImGui::SetNextWindowSize(ImVec2(-1,-1));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoSavedSettings;


    ImGui::Begin(ICON_FA_PALETTE "###Tools", nullptr, flags);
  //  ImGui::PopStyleVar();
    ImGui::Button(ICON_FA_PAINTBRUSH);
    ImGui::End();
}

void TerrainDocument::OnCreated()
{
    Camera.fovy = 45;
    Camera.up.z = 1;
    Camera.position.z = 100;
    Camera.target.y = 50;
}