#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

struct FrustumPlane
{
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;

    void Normalize()
    {
        float len = sqrtf(a * a + b * b + c * c);
        if (len > 0.0f)
        {
            float inv = 1.0f / len;
            a *= inv;
            b *= inv;
            c *= inv;
            d *= inv;
        }
    }

    inline float DistanceToPoint(const Vector3& p) const
    {
        return a * p.x + b * p.y + c * p.z + d;
    }
};

struct Frustum
{
    // 0: Left, 1: Right, 2: Bottom, 3: Top, 4: Near, 5: Far
    FrustumPlane Planes[6];

    // Extract frustum planes from a combined View-Projection matrix
    static Frustum ExtractFromMatrix(const Matrix& matVP)
    {
        Frustum f;

        // Left plane: wc + xc >= 0
        f.Planes[0].a = matVP.m3 + matVP.m0;
        f.Planes[0].b = matVP.m7 + matVP.m4;
        f.Planes[0].c = matVP.m11 + matVP.m8;
        f.Planes[0].d = matVP.m15 + matVP.m12;

        // Right plane: wc - xc >= 0
        f.Planes[1].a = matVP.m3 - matVP.m0;
        f.Planes[1].b = matVP.m7 - matVP.m4;
        f.Planes[1].c = matVP.m11 - matVP.m8;
        f.Planes[1].d = matVP.m15 - matVP.m12;

        // Bottom plane: wc + yc >= 0
        f.Planes[2].a = matVP.m3 + matVP.m1;
        f.Planes[2].b = matVP.m7 + matVP.m5;
        f.Planes[2].c = matVP.m11 + matVP.m9;
        f.Planes[2].d = matVP.m15 + matVP.m13;

        // Top plane: wc - yc >= 0
        f.Planes[3].a = matVP.m3 - matVP.m1;
        f.Planes[3].b = matVP.m7 - matVP.m5;
        f.Planes[3].c = matVP.m11 - matVP.m9;
        f.Planes[3].d = matVP.m15 - matVP.m13;

        // Near plane: wc + zc >= 0
        f.Planes[4].a = matVP.m3 + matVP.m2;
        f.Planes[4].b = matVP.m7 + matVP.m6;
        f.Planes[4].c = matVP.m11 + matVP.m10;
        f.Planes[4].d = matVP.m15 + matVP.m14;

        // Far plane: wc - zc >= 0
        f.Planes[5].a = matVP.m3 - matVP.m2;
        f.Planes[5].b = matVP.m7 - matVP.m6;
        f.Planes[5].c = matVP.m11 - matVP.m10;
        f.Planes[5].d = matVP.m15 - matVP.m14;

        for (int i = 0; i < 6; i++)
            f.Planes[i].Normalize();

        return f;
    }

    // Extract frustum planes using current active modelview and projection matrices in rlgl (between BeginMode3D and EndMode3D)
    static Frustum ExtractFromCurrent()
    {
        Matrix matView = rlGetMatrixModelview();
        Matrix matProj = rlGetMatrixProjection();
        return ExtractFromMatrix(MatrixMultiply(matView, matProj));
    }

    // Check if a point is within the frustum
    inline bool IsPointVisible(const Vector3& point) const
    {
        for (int i = 0; i < 6; i++)
        {
            if (Planes[i].DistanceToPoint(point) < 0.0f)
                return false;
        }
        return true;
    }

    // Check if an axis-aligned bounding box intersects or is inside the frustum
    inline bool IsBoxVisible(const BoundingBox& box) const
    {
        for (int i = 0; i < 6; i++)
        {
            const auto& p = Planes[i];
            Vector3 pos;
            pos.x = (p.a > 0.0f) ? box.max.x : box.min.x;
            pos.y = (p.b > 0.0f) ? box.max.y : box.min.y;
            pos.z = (p.c > 0.0f) ? box.max.z : box.min.z;

            if (p.DistanceToPoint(pos) < 0.0f)
                return false;
        }
        return true;
    }

    // Check if an axis-aligned bounding box defined by min and max corners is inside the frustum
    inline bool IsBoxVisible(const Vector3& min, const Vector3& max) const
    {
        return IsBoxVisible(BoundingBox{ min, max });
    }
};
