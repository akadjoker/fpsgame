#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <rlgl.h>
#include <vector>
 

 

struct Triangle;  

struct DecalData
{
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    float size;
    Texture2D texture;
    Color tint;
    bool active;
    u32 ID;

    DecalData();
};

class Decal3D 
{
private:
    std::vector<DecalData> decals;
    int maxDecals;
    int currentDecalIndex;

    void DrawTexturedQuad(Texture2D texture, Vector3 center, Vector3 tangent,
                          Vector3 bitangent, float size, Color tint, float move = 0.005f);

public:
    Decal3D(int maxDecals = 100);

    void AddDecal(Vector3 position, Vector3 normal, const Triangle* tri,
                  Texture2D decalTexture, float decalSize = 1.0f,
                  Color tint = WHITE, int ID = 0);

    void Draw(bool debug = false);

    void ClearDecals();
    void RemoveLastDecal();
    void ClearByID(u32 id);

    int GetActiveDecalCount() const;
};
