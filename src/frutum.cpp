#include "frustum.hpp"

void ViewFrustum::update()
{
    Matrix projection = rlGetMatrixProjection();
    Matrix modelview = rlGetMatrixModelview();

    Matrix planes = { 0 };

    planes.m0 = modelview.m0 * projection.m0 + modelview.m1 * projection.m4 + modelview.m2 * projection.m8 + modelview.m3 * projection.m12;
    planes.m1 = modelview.m0 * projection.m1 + modelview.m1 * projection.m5 + modelview.m2 * projection.m9 + modelview.m3 * projection.m13;
    planes.m2 = modelview.m0 * projection.m2 + modelview.m1 * projection.m6 + modelview.m2 * projection.m10 + modelview.m3 * projection.m14;
    planes.m3 = modelview.m0 * projection.m3 + modelview.m1 * projection.m7 + modelview.m2 * projection.m11 + modelview.m3 * projection.m15;
    planes.m4 = modelview.m4 * projection.m0 + modelview.m5 * projection.m4 + modelview.m6 * projection.m8 + modelview.m7 * projection.m12;
    planes.m5 = modelview.m4 * projection.m1 + modelview.m5 * projection.m5 + modelview.m6 * projection.m9 + modelview.m7 * projection.m13;
    planes.m6 = modelview.m4 * projection.m2 + modelview.m5 * projection.m6 + modelview.m6 * projection.m10 + modelview.m7 * projection.m14;
    planes.m7 = modelview.m4 * projection.m3 + modelview.m5 * projection.m7 + modelview.m6 * projection.m11 + modelview.m7 * projection.m15;
    planes.m8 = modelview.m8 * projection.m0 + modelview.m9 * projection.m4 + modelview.m10 * projection.m8 + modelview.m11 * projection.m12;
    planes.m9 = modelview.m8 * projection.m1 + modelview.m9 * projection.m5 + modelview.m10 * projection.m9 + modelview.m11 * projection.m13;
    planes.m10 = modelview.m8 * projection.m2 + modelview.m9 * projection.m6 + modelview.m10 * projection.m10 + modelview.m11 * projection.m14;
    planes.m11 = modelview.m8 * projection.m3 + modelview.m9 * projection.m7 + modelview.m10 * projection.m11 + modelview.m11 * projection.m15;
    planes.m12 = modelview.m12 * projection.m0 + modelview.m13 * projection.m4 + modelview.m14 * projection.m8 + modelview.m15 * projection.m12;
    planes.m13 = modelview.m12 * projection.m1 + modelview.m13 * projection.m5 + modelview.m14 * projection.m9 + modelview.m15 * projection.m13;
    planes.m14 = modelview.m12 * projection.m2 + modelview.m13 * projection.m6 + modelview.m14 * projection.m10 + modelview.m15 * projection.m14;
    planes.m15 = modelview.m12 * projection.m3 + modelview.m13 * projection.m7 + modelview.m14 * projection.m11 + modelview.m15 * projection.m15;

    Planes[Right]  = { planes.m3 - planes.m0, planes.m7 - planes.m4, planes.m11 - planes.m8, planes.m15 - planes.m12 };
    NormalizePlane(&Planes[Right]);

    Planes[Left]   = { planes.m3 + planes.m0, planes.m7 + planes.m4, planes.m11 + planes.m8, planes.m15 + planes.m12 };
    NormalizePlane(&Planes[Left]);

    Planes[Top]    = { planes.m3 - planes.m1, planes.m7 - planes.m5, planes.m11 - planes.m9, planes.m15 - planes.m13 };
    NormalizePlane(&Planes[Top]);

    Planes[Bottom] = { planes.m3 + planes.m1, planes.m7 + planes.m5, planes.m11 + planes.m9, planes.m15 + planes.m13 };
    NormalizePlane(&Planes[Bottom]);

    Planes[Back]   = { planes.m3 - planes.m2, planes.m7 - planes.m6, planes.m11 - planes.m10, planes.m15 - planes.m14 };
    NormalizePlane(&Planes[Back]);

    Planes[Front]  = { planes.m3 + planes.m2, planes.m7 + planes.m6, planes.m11 + planes.m10, planes.m15 + planes.m14 };
    NormalizePlane(&Planes[Front]);
}

bool ViewFrustum::isSphereInside(const Vector3& position, float radius)
{
    for (int i = 0; i < 6; i++)
    {
        if (DistanceToPlaneV(&Planes[i], &position) < -radius)
            return false;
    }
    return true;
}

bool ViewFrustum::isBoxInside(const BoundingBox& box)
{
    return isMinMaxInside(box.min, box.max);
}

bool ViewFrustum::isMinMaxInside(const Vector3& min, const Vector3& max)
{
    if (isPointInside(min.x, min.y, min.z)) return true;
    if (isPointInside(min.x, max.y, min.z)) return true;
    if (isPointInside(max.x, max.y, min.z)) return true;
    if (isPointInside(max.x, min.y, min.z)) return true;
    if (isPointInside(min.x, min.y, max.z)) return true;
    if (isPointInside(min.x, max.y, max.z)) return true;
    if (isPointInside(max.x, max.y, max.z)) return true;
    if (isPointInside(max.x, min.y, max.z)) return true;

    for (int i = 0; i < 6; i++)
    {
        bool oneInside = false;

        if (DistanceToPlane(&Planes[i], min.x, min.y, min.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], max.x, min.y, min.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], max.x, max.y, min.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], min.x, max.y, min.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], min.x, min.y, max.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], max.x, min.y, max.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], max.x, max.y, max.z) >= 0) oneInside = true;
        if (DistanceToPlane(&Planes[i], min.x, max.y, max.z) >= 0) oneInside = true;

        if (!oneInside) return false;
    }
    return true;
}

bool ViewFrustum::isPointInside(const Vector3& position)
{
    for (int i = 0; i < 6; i++)
    {
        if (DistanceToPlaneV(&Planes[i], &position) <= 0)
            return false;
    }
    return true;
}

bool ViewFrustum::isPointInside(float x, float y, float z)
{
    for (int i = 0; i < 6; i++)
    {
        if (DistanceToPlane(&Planes[i], x, y, z) <= 0)
            return false;
    }
    return true;
}

void ViewFrustum::NormalizePlane(Vector4* plane)
{
    if (plane == NULL) return;
    float magnitude = sqrtf(plane->x * plane->x + plane->y * plane->y + plane->z * plane->z);
    plane->x /= magnitude;
    plane->y /= magnitude;
    plane->z /= magnitude;
    plane->w /= magnitude;
}

float ViewFrustum::DistanceToPlaneV(const Vector4* plane, const Vector3* position)
{
    return (plane->x * position->x + plane->y * position->y + plane->z * position->z + plane->w);
}

float ViewFrustum::DistanceToPlane(const Vector4* plane, float x, float y, float z)
{
    return (plane->x * x + plane->y * y + plane->z * z + plane->w);
}
