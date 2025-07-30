 
#include "decal.hpp"
#include "collision.hpp"


DecalData::DecalData()
    : position({0}), normal({0, 1, 0}), tangent({1, 0, 0}),
      bitangent({0, 0, 1}), size(1.0f), tint(WHITE), active(false), ID(0)
{}

Decal3D::Decal3D(int maxDecals) : maxDecals(maxDecals), currentDecalIndex(0)
{
    decals.resize(maxDecals);
}

void Decal3D::AddDecal(Vector3 position, Vector3 normal, const Triangle* tri,
                       Texture2D decalTexture, float decalSize, Color tint, int ID)
{
    DecalData& decal = decals[currentDecalIndex];
    decal.ID = ID;
    decal.position = position;
    decal.normal = Vector3Normalize(normal);

    // Precisa de implementar a função CalculateSurfaceOrientation()
    CalculateSurfaceOrientation(position, tri, decal.tangent, decal.bitangent, decal.normal);

    decal.size = decalSize;
    decal.texture = decalTexture;
    decal.tint = tint;
    decal.active = true;

    currentDecalIndex = (currentDecalIndex + 1) % maxDecals;
}

void Decal3D::DrawTexturedQuad(Texture2D texture, Vector3 center, Vector3 tangent,
                               Vector3 bitangent, float size, Color tint, float move)
{
    Vector3 halfTangent = Vector3Scale(tangent, size * 0.5f);
    Vector3 halfBitangent = Vector3Scale(bitangent, size * 0.5f);

    Vector3 v1 = Vector3Add(Vector3Subtract(center, halfTangent), halfBitangent);
    Vector3 v2 = Vector3Add(Vector3Add(center, halfTangent), halfBitangent);
    Vector3 v3 = Vector3Add(Vector3Add(center, halfTangent), Vector3Negate(halfBitangent));
    Vector3 v4 = Vector3Add(Vector3Subtract(center, halfTangent), Vector3Negate(halfBitangent));

    Vector3 offset = Vector3Scale(Vector3Normalize(Vector3CrossProduct(tangent, bitangent)), move);
    v1 = Vector3Add(v1, offset);
    v2 = Vector3Add(v2, offset);
    v3 = Vector3Add(v3, offset);
    v4 = Vector3Add(v4, offset);

    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(v1.x, v1.y, v1.z);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(v2.x, v2.y, v2.z);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(v3.x, v3.y, v3.z);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(v4.x, v4.y, v4.z);

    rlEnd();
}

void Decal3D::Draw(bool debug)
{
    for (const DecalData& decal : decals)
    {
        if (!decal.active) continue;
        DrawTexturedQuad(decal.texture, decal.position, decal.tangent,
                         decal.bitangent, decal.size, decal.tint);

        if (debug)
        {
            Vector3 normalEnd = Vector3Add(decal.position, Vector3Scale(decal.normal, 0.5f));
            Vector3 tangentEnd = Vector3Add(decal.position, Vector3Scale(decal.tangent, 0.5f));
            Vector3 bitangentEnd = Vector3Add(decal.position, Vector3Scale(decal.bitangent, 0.5f));
            DrawLine3D(decal.position, normalEnd, BLUE);
            DrawLine3D(decal.position, tangentEnd, RED);
            DrawLine3D(decal.position, bitangentEnd, GREEN);
        }
    }

    rlSetTexture(0);
}

void Decal3D::ClearDecals()
{
    for (DecalData& decal : decals) decal.active = false;
    currentDecalIndex = 0;
}

void Decal3D::RemoveLastDecal()
{
    if (currentDecalIndex > 0)
    {
        currentDecalIndex--;
        decals[currentDecalIndex].active = false;
    }
    else
    {
        for (int i = maxDecals - 1; i >= 0; i--)
        {
            if (decals[i].active)
            {
                decals[i].active = false;
                currentDecalIndex = i;
                break;
            }
        }
    }
}

int Decal3D::GetActiveDecalCount() const
{
    int count = 0;
    for (const DecalData& decal : decals)
        if (decal.active) count++;
    return count;
}

void Decal3D::ClearByID(u32 id)
{
    for (auto& decal : decals)
    {
        if (decal.ID == id)
        {
            decal.active = false;
        }
    }
}
