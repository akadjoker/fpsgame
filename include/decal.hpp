#pragma once
// #include "Config.hpp"
// #include <raylib.h>
// #include <raymath.h>
// #include <rlgl.h>
// #include <vector>
// #include <algorithm>

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

    DecalData()
        : position({ 0 }), normal({ 0, 1, 0 }), tangent({ 1, 0, 0 }),
          bitangent({ 0, 0, 1 }), size(1.0f), tint(WHITE), active(false)
    {}
};

class Decal3D 
{
private:
    std::vector<DecalData> decals;
    int maxDecals;
    int currentDecalIndex;

 
 

public:
    Decal3D(int maxDecals = 100): maxDecals(maxDecals), currentDecalIndex(0)
    {
        decals.resize(maxDecals);
    }
 
      void AddDecal(Vector3 position, Vector3 normal,const Triangle* tri , Texture2D decalTexture, float decalSize = 1.0f,Color tint = WHITE)
    {
        DecalData& decal = decals[currentDecalIndex];
        decal.position = position;
        decal.normal = Vector3Normalize(normal);

        CalculateSurfaceOrientation(position, tri, decal.tangent, decal.bitangent, decal.normal);
  
        decal.size = decalSize;
        decal.texture = decalTexture;
        decal.tint = tint;
        decal.active = true;

        currentDecalIndex = (currentDecalIndex + 1) % maxDecals;
    }
 

 

void DrawTexturedQuad(Texture2D texture, Vector3 center, Vector3 tangent, Vector3 bitangent, float size, Color tint,float move= 0.005f)
{
    Vector3 halfTangent = Vector3Scale(tangent, size * 0.5f);
    Vector3 halfBitangent = Vector3Scale(bitangent, size * 0.5f);

    Vector3 v1 = Vector3Add(Vector3Subtract(center, halfTangent), halfBitangent);
    Vector3 v2 = Vector3Add(Vector3Add(center, halfTangent), halfBitangent);
    Vector3 v3 = Vector3Add(Vector3Add(center, halfTangent), Vector3Negate(halfBitangent));
    Vector3 v4 = Vector3Add(Vector3Subtract(center, halfTangent), Vector3Negate(halfBitangent));

    // Offset ligeiro na normal (para evitar z-fighting)
    Vector3 offset = Vector3Scale(Vector3Normalize(Vector3CrossProduct(tangent, bitangent)), move);
    v1 = Vector3Add(v1, offset);
    v2 = Vector3Add(v2, offset);
    v3 = Vector3Add(v3, offset);
    v4 = Vector3Add(v4, offset);

    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    // rlTexCoord2f(0.0f, 1.0f); rlVertex3f(v1.x, v1.y, v1.z);
    // rlTexCoord2f(1.0f, 1.0f); rlVertex3f(v2.x, v2.y, v2.z);
    // rlTexCoord2f(1.0f, 0.0f); rlVertex3f(v3.x, v3.y, v3.z);
    // rlTexCoord2f(0.0f, 0.0f); rlVertex3f(v4.x, v4.y, v4.z);

    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(v1.x, v1.y, v1.z); // Bottom-left
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(v2.x, v2.y, v2.z); // Bottom-right  
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(v3.x, v3.y, v3.z); // Top-right
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(v4.x, v4.y, v4.z); // Top-left

    rlEnd();

}



    void Draw(bool debug =false)
    {

    
 
        for (const DecalData& decal : decals)
        {
            if (!decal.active) continue;
            DrawTexturedQuad(decal.texture, decal.position, decal.tangent, decal.bitangent, decal.size, decal.tint);

        
            if (debug)
            {
            Vector3 normalEnd = Vector3Add(decal.position, Vector3Scale(decal.normal, 0.5f));
            DrawLine3D(decal.position, normalEnd, BLUE);
            Vector3 tangentEnd = Vector3Add(decal.position, Vector3Scale(decal.tangent, 0.5f));
            Vector3 bitangentEnd = Vector3Add(decal.position, Vector3Scale(decal.bitangent, 0.5f));
            DrawLine3D(decal.position, tangentEnd, RED);       // Tangent em vermelho
            DrawLine3D(decal.position, bitangentEnd, GREEN);   // Bitangent em verde
            }
        }

       
       rlSetTexture(0);
            
    }

    // Remove todos os decals
    void ClearDecals()
    {
        for (DecalData& decal : decals)
        {
            decal.active = false;
        }
        currentDecalIndex = 0;
    }

    // Remove o último decal adicionado
    void RemoveLastDecal()
    {
        if (currentDecalIndex > 0)
        {
            currentDecalIndex--;
            decals[currentDecalIndex].active = false;
        }
        else
        {
            // Se estamos no início, procura o último ativo
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

    // Obtém o número de decals ativos
    int GetActiveDecalCount() const
    {
        int count = 0;
        for (const DecalData& decal : decals)
        {
            if (decal.active) count++;
        }
        return count;
    }

    // Define a cor de tint para próximos decals
    void SetDecalTint(Color tint)
    {
        // Esta função pode ser expandida para aplicar tint a decals específicos
    }
};

// Exemplo de uso:
/*
int main() {
    InitWindow(800, 600, "3D Decals Example");

    Camera camera = { 0 };
    camera.position = {0.0f, 10.0f, 10.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Model model = LoadModel("model.obj");
    Texture2D decalTexture = LoadTexture("decal.png");

    Decal3D decalSystem(50);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            decalSystem.AddDecalAtMousePos(model, camera, decalTexture, 1.0f);
        }

        if (IsKeyPressed(KEY_C)) {
            decalSystem.ClearDecals();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawModel(model, {0, 0, 0}, 1.0f, WHITE);
        decalSystem.DrawDecals();
        EndMode3D();

        DrawText("Left click to add decal, C to clear", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    UnloadModel(model);
    UnloadTexture(decalTexture);
    CloseWindow();

    return 0;
}
*/