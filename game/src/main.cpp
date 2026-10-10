/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you
--  wrote the original software. If you use this software in a product, an acknowledgment
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rcamera.h"

#include "game.h"   // an external header in this project


#if defined(_WIN32)
#include "assert.h"
#else
void IGNORE(const char*) {}
#define _wassert (F, M) F IGNORE(M);
#define _ASSERT (F, M) F IGNORE(M);
#endif


int LODLevel = 0;

#include "TerrainTile.h"
#include "TerrainBuilder.h"
#include "TerrainRender.h"

#include <algorithm>
#include <cmath>

static constexpr float CAMERA_HEIGHT_OFFSET = 2.0f;


float SunVector[3] = { 0,0,1 };
int SunVectorLoc = 0;

TerrainInfo info;

TerrainMaterial GrassMateral;
TerrainMaterial GroundMateral;
TerrainMaterial RoadMateral;
TerrainMaterial SnowMateral;

std::vector<TerrainTile> Tiles;

struct CameraState
{
	Vector2 ViewAngles = { 0,0 };
	Vector3 Position = { 0,0,0 };

    float Offset = CAMERA_HEIGHT_OFFSET;
	Camera3D ViewCamera = { 0 };
};

CameraState ViewCameraState;

TerainRenderer Renderer;

Shader TerrainShader = { 0 };

static constexpr float CAMERA_MOVE_SPEED = 20;
static constexpr float CAMERA_ROTATION_SPEED = 1;
static constexpr float CAMERA_PAN_SPEED = 20;
static constexpr float CAMERA_ORBITAL_SPEED = 0.25f;
static constexpr float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.003f;

float GetTerrainHeight(float worldX, float worldY);

void UpdateCamera2()
{
    ViewCameraState.ViewAngles.x -= GetMouseDelta().x * CAMERA_MOUSE_MOVE_SENSITIVITY;
    ViewCameraState.ViewAngles.y -= GetMouseDelta().y * CAMERA_MOUSE_MOVE_SENSITIVITY;
    if (ViewCameraState.ViewAngles.y > 1.5f)
        ViewCameraState.ViewAngles.y = 1.5f;
    if (ViewCameraState.ViewAngles.y < -1.5f)
        ViewCameraState.ViewAngles.y = -1.5f;
    Vector3 forward = { cosf(ViewCameraState.ViewAngles.x) * cosf(ViewCameraState.ViewAngles.y), sinf(ViewCameraState.ViewAngles.x) * cosf(ViewCameraState.ViewAngles.y), sinf(ViewCameraState.ViewAngles.y) };
    Vector3 forwardFlat = { cosf(ViewCameraState.ViewAngles.x), sinf(ViewCameraState.ViewAngles.x), 0 };
	
	Vector3 right = { -sinf(ViewCameraState.ViewAngles.x), cosf(ViewCameraState.ViewAngles.x), 0 };


    float cameraMoveSpeed = CAMERA_MOVE_SPEED * GetFrameTime();
    if (IsKeyDown(KEY_LEFT_SHIFT))
        cameraMoveSpeed *= 10;
    if (IsKeyDown(KEY_W)) ViewCameraState.Position = Vector3Add(ViewCameraState.Position, Vector3Scale(forwardFlat, cameraMoveSpeed));
    if (IsKeyDown(KEY_S)) ViewCameraState.Position = Vector3Subtract(ViewCameraState.Position, Vector3Scale(forwardFlat, cameraMoveSpeed));
    if (IsKeyDown(KEY_A)) ViewCameraState.Position = Vector3Subtract(ViewCameraState.Position, Vector3Scale(right, cameraMoveSpeed));
    if (IsKeyDown(KEY_D)) ViewCameraState.Position = Vector3Add(ViewCameraState.Position, Vector3Scale(right, cameraMoveSpeed));
    if (IsKeyDown(KEY_Q)) ViewCameraState.Offset -= cameraMoveSpeed;
    if (IsKeyDown(KEY_E)) ViewCameraState.Offset += cameraMoveSpeed;

    float terrainHeight = GetTerrainHeight(ViewCameraState.Position.x, ViewCameraState.Position.y);
    float desiredZ = terrainHeight + ViewCameraState.Offset;
    ViewCameraState.Position.z = desiredZ;

	ViewCameraState.ViewCamera.position = ViewCameraState.Position;
	ViewCameraState.ViewCamera.target = Vector3Add(ViewCameraState.ViewCamera.position, forward);
}

void UpdateCameraXY(Camera* camera, int mode)
{
	Vector2 mousePositionDelta = GetMouseDelta();

	bool moveInWorldPlane = true;
	bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
	bool lockView = ((mode == CAMERA_FREE) || (mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
	bool rotateUp = false;

	// Camera speeds based on frame time
	float cameraMoveSpeed = CAMERA_MOVE_SPEED * GetFrameTime();
	if (IsKeyDown(KEY_LEFT_SHIFT))
		cameraMoveSpeed *= 10;

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

void GameInit()
{
	SetConfigFlags(/*FLAG_VSYNC_HINT |*/ FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 720, "Example");
	SetTargetFPS(144);

	TerrainShader = LoadShader("resources/shaders/terrain.vs", "resources/shaders/terrain.fs");
	if (!IsShaderValid(TerrainShader))
		TerrainShader = LoadShader(nullptr, nullptr);

	SunVectorLoc = GetShaderLocation(TerrainShader, "sunVector");

	int selected = 0;
	SetShaderValue(TerrainShader, GetShaderLocation(TerrainShader, "selected"), &selected, SHADER_UNIFORM_INT);
	int showSplat = 0;
	SetShaderValue(TerrainShader, GetShaderLocation(TerrainShader, "showSplat"), &showSplat, SHADER_UNIFORM_INT);

	rlSetClipPlanes(0.1f, 5000.0f);

	float perlinScale = 2;

	info.TerrainMinZ = -6;
	info.TerrainMaxZ = 25;

	GrassMateral.DiffuseMap = LoadTextureFromImage(GenImageChecked(128, 128, 32, 32, DARKGREEN, GREEN));
	GenTextureMipmaps(&GrassMateral.DiffuseMap);
	SetTextureFilter(GrassMateral.DiffuseMap, TEXTURE_FILTER_TRILINEAR);

	GroundMateral.DiffuseMap = LoadTextureFromImage(GenImageChecked(128, 128, 32, 32, DARKBROWN, BROWN));
	GenTextureMipmaps(&GroundMateral.DiffuseMap);
	SetTextureFilter(GroundMateral.DiffuseMap, TEXTURE_FILTER_TRILINEAR);

	RoadMateral.DiffuseMap = LoadTextureFromImage(GenImageColor(128, 128, YELLOW));
	GenTextureMipmaps(&RoadMateral.DiffuseMap);
	SetTextureFilter(RoadMateral.DiffuseMap, TEXTURE_FILTER_TRILINEAR);

	SnowMateral.DiffuseMap = LoadTextureFromImage(GenImageColor(128, 128, PURPLE));
	GenTextureMipmaps(&SnowMateral.DiffuseMap);
	SetTextureFilter(SnowMateral.DiffuseMap, TEXTURE_FILTER_TRILINEAR);

	TileMeshBuilder builder;

    int grid = 8;

    Image testSplat = GenImageChecked(65, 65, 2, 2, Color{ 255,0,0,0 }, Color{ 0,255,0,0 });
    ImageDrawRectangle(&testSplat, 16, 16, 32, 32, Color{ 0,0,0,0 });
	//ImageDrawCircle(&testSplat, 32, 32, 8, Color{ 0,0,0,0 });
	
    Texture testSplatTexture = LoadTextureFromImage(testSplat);
	UnloadImage(testSplat);

    double startTime = GetTime();

	for (int y = -grid; y < grid; y++)
	{
		for (int x = -grid; x < grid; x++)
		{
			auto& tile = Tiles.emplace_back(info);
			Image heightmap = GenImagePerlinNoise(131, 131, (x * 128) - 1, (y * 128) - 1, perlinScale);
			tile.SetHeightsFromImage(heightmap);
			UnloadImage(heightmap);

			tile.Splatmap = testSplatTexture;
			
			tile.LayerMaterials.push_back(&GrassMateral);
			tile.LayerMaterials.push_back(&GroundMateral);
			tile.LayerMaterials.push_back(&RoadMateral);

			tile.Origin = TerrainPosition{ x, y };
			builder.Build(tile);
		}
	}

	double endTime = GetTime();
	TraceLog(LOG_INFO, "Terrain generation time: %f ms", (endTime - startTime) * 1000);

	Renderer.SetShader(TerrainShader);

	ViewCameraState.ViewCamera.fovy = 45;
	ViewCameraState.Position.z = 10;
	ViewCameraState.Position.y = -10;

	ViewCameraState.ViewCamera.up.z = 1;
	UpdateCamera2();
	// load resources
}

float GetTerrainHeight(float worldX, float worldY)
{
	if (Tiles.empty())
		return 0.0f;

	int64_t minTileX = INT64_MAX, maxTileX = INT64_MIN;
	int64_t minTileY = INT64_MAX, maxTileY = INT64_MIN;
	for (const auto& t : Tiles)
	{
		minTileX = std::min(minTileX, t.Origin.X);
		maxTileX = std::max(maxTileX, t.Origin.X);
		minTileY = std::min(minTileY, t.Origin.Y);
		maxTileY = std::max(maxTileY, t.Origin.Y);
	}

	float minWorldX = minTileX * info.TerrainTileSize;
	float maxWorldX = (maxTileX + 1) * info.TerrainTileSize;
	float minWorldY = minTileY * info.TerrainTileSize;
	float maxWorldY = (maxTileY + 1) * info.TerrainTileSize;

	float clampedX = std::clamp(worldX, minWorldX, maxWorldX);
	float clampedY = std::clamp(worldY, minWorldY, maxWorldY);

	int64_t tileX = (int64_t)std::floor(clampedX / info.TerrainTileSize);
	int64_t tileY = (int64_t)std::floor(clampedY / info.TerrainTileSize);
	if (tileX > maxTileX) tileX = maxTileX;
	if (tileY > maxTileY) tileY = maxTileY;

	for (const auto& t : Tiles)
	{
		if (t.Origin.X == tileX && t.Origin.Y == tileY)
		{
			float localX = clampedX - (t.Origin.X * info.TerrainTileSize);
			float localY = clampedY - (t.Origin.Y * info.TerrainTileSize);
			return t.GetHeightAt(localX, localY);
		}
	}

	return 0.0f;
}

void GameCleanup()
{
	// unload resources

	CloseWindow();
}

bool GameUpdate()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		UpdateCamera2();

	if (IsKeyDown(KEY_ONE))
		LODLevel = 0;
	if (IsKeyDown(KEY_TWO))
		LODLevel = 1;
	if (IsKeyDown(KEY_THREE))
		LODLevel = 2;
	if (IsKeyDown(KEY_FOUR))
		LODLevel = 3;

	SunVector[0] = sinf(float(GetTime()) / 4);
	SunVector[1] = cosf(float(GetTime()) / 1.4f);
	SunVector[1] = abs(cosf(float(GetTime()) / 2.4f));

	SetShaderValue(TerrainShader, SunVectorLoc, SunVector, SHADER_UNIFORM_VEC3);
	return true;
}

float Lod0Color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float Lod1Color[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
float Lod2Color[4] = { 0.5f, 1.0f, 0.5f, 1.0f };
float Lod3Color[4] = { 0.5f, 0.5f, 1.0f, 1.0f };


int GetLODLevel(const TerrainPosition& tilePos, Vector3& cameraPos)
{
    int x = int((cameraPos.x) / info.TerrainGridSize);
	int y = int((cameraPos.y)/ info.TerrainGridSize);

	int lodStep = 2;

	int lod = (int)std::max(std::abs(tilePos.X - x), std::abs(tilePos.Y - y)) / lodStep;

	return lod;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(BLACK);

	BeginMode3D(ViewCameraState.ViewCamera);

	rlPushMatrix();
	rlRotatef(90, 1, 0, 0);
	//  DrawGrid(200, 1);
	rlPopMatrix();

	DrawCube(Vector3Zeros, 1, 1, 1, RED);
	DrawCube(Vector3{ info.TerrainGridSize * 1.0f, info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
	DrawCube(Vector3{ 0, info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
	DrawCube(Vector3{ info.TerrainGridSize * 1.0f,0,0 }, 1, 1, 1, BLUE);

	DrawCube(Vector3{ 0,info.TerrainGridSize * 0.5f,0 }, 0.5f, 0.5f, 0.5f, YELLOW);
	DrawCube(Vector3{ info.TerrainGridSize * 0.5f,0,0 }, 0.5f, 0.5f, 0.5f, YELLOW);

	DrawCube(Vector3{ 0,1,0 }, 0.125f, 2, 0.125f, PURPLE);
	
	//rlEnableWireMode();
	for (int i = 0; i < Tiles.size(); i++)
	{
        int lod = GetLODLevel(Tiles[i].Origin, ViewCameraState.Position);
        lod = std::max(LODLevel, lod);
		if (lod >= MaxLODLevels)
			lod = MaxLODLevels - 1;

		auto GetNeighborLOD = [&](int64_t x, int64_t y) -> int {
			for (const auto& t : Tiles)
			{
				if (t.Origin.X == x && t.Origin.Y == y)
				{
					int nl = GetLODLevel(t.Origin, ViewCameraState.Position);
					nl = std::max(LODLevel, nl);
					if (nl >= MaxLODLevels) 
						nl = MaxLODLevels - 1;
					return nl;
				}
			}
			return -1;
		};

		uint8_t transitionMask = TRANSITION_NONE;
		int northLOD = GetNeighborLOD(Tiles[i].Origin.X, Tiles[i].Origin.Y + 1);
		int southLOD = GetNeighborLOD(Tiles[i].Origin.X, Tiles[i].Origin.Y - 1);
		int eastLOD  = GetNeighborLOD(Tiles[i].Origin.X + 1, Tiles[i].Origin.Y);
		int westLOD  = GetNeighborLOD(Tiles[i].Origin.X - 1, Tiles[i].Origin.Y);

		if (northLOD > lod) transitionMask |= TRANSITION_NORTH;
		if (southLOD > lod) transitionMask |= TRANSITION_SOUTH;
		if (eastLOD  > lod) transitionMask |= TRANSITION_EAST;
		if (westLOD  > lod) transitionMask |= TRANSITION_WEST;

		float* color = Lod0Color;
		if (lod == 1)
			color = Lod1Color;
		else if (lod == 2)
			color = Lod2Color;
		else if (lod == 3)
			color = Lod3Color;

		Renderer.Draw(Tiles[i], lod, transitionMask, color);

	}
	//rlDisableWireMode();

	EndMode3D();

	DrawText(TextFormat("LOD Level = %d", LODLevel), 3, 20, 20, WHITE);
	DrawText(TextFormat("Camera Pos: (%.1f, %.1f, %.1f)", ViewCameraState.Position.x, ViewCameraState.Position.y, ViewCameraState.Position.z), 3, 40, 20, WHITE);
	DrawText(TextFormat("Terrain Height: %.2f (+%.1f offset)", GetTerrainHeight(ViewCameraState.Position.x, ViewCameraState.Position.y), ViewCameraState.Offset), 3, 60, 20, WHITE);
	DrawFPS(3, 3);
	EndDrawing();
}

int main()
{
	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;

		GameDraw();
	}
	GameCleanup();

	return 0;
}