#pragma once

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cmath>

enum FrustumPlanes
{
    Back = 0,
    Front = 1,
    Bottom = 2,
    Top = 3,
    Right = 4,
    Left = 5,
    MAX = 6
};

class ViewFrustum 
{
public:
    Vector4 Planes[6];

    void update();

    bool isSphereInside(const Vector3& position, float radius);
    bool isBoxInside(const BoundingBox& box);
    bool isMinMaxInside(const Vector3& min, const Vector3& max);
    bool isPointInside(const Vector3& position);
    bool isPointInside(float x, float y, float z);

private:
    void NormalizePlane(Vector4* plane);
    float DistanceToPlaneV(const Vector4* plane, const Vector3* position);
    float DistanceToPlane(const Vector4* plane, float x, float y, float z);
};
