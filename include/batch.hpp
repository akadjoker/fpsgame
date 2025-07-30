#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <cstdint>


class Batch {
private:
    u32 vaoId;
    u32 vboId[4];
    std::vector<Vector3> vertices;
    std::vector<Vector2> textcoord;
    std::vector<u8> colors;
    std::vector<u16> indices;
    u32 maxVertices;
    u32 maxIndices;
    u32 maxQuads;
    u32 vertexCount{ 0 };
    u32 indexCount{ 0 };
    u32 quadCount{ 0 };
    bool dynamic = false;
    bool isBegin{ false };

public:
    void Init(u32 maxQuads, bool dynamic = true);
    void Release();

    void BeginQuad();
    void AddVertex(Vector3 position, Vector2 texCoords, Color color);
    void AddVertex(float x, float y, float z, float u, float v, u8 r, u8 g,
                   u8 b, u8 a);
    void EndQuad();

    void AddQuad(Vector3 position, Vector2 size, Vector2 texCoords[4],
                 Color color);
    void AddQuad(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3, Color color,
                 Vector2 t0, Vector2 t1, Vector2 t2, Vector2 t3);
    void AddQuad(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3, Color color);
    void AddQuad(float x, float y, float width, float height, Color color);

    void SetVertex(float x, float y, float z, float u, float v, u8 r, u8 g,
                   u8 b, u8 a);
    void Flush();
    void Render(u32 texture);
    void Clear();

    u32 GetQuadCount() const;
    u32 GetVertexCount() const;
    bool IsFull() const;

    void DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position,
                           float scale, Color tint);
    void DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position,
                           Vector2 size, float angle, Color tint);
    void DrawQuadBillboard(Matrix& matView, Texture& texture, Rectangle source,
                           Vector3 position, Vector2 size, Color tint);
    void DrawQuadBillboard(Matrix& matView, Texture& texture, Rectangle source,
                           Vector3 position, Vector3 up, Vector2 size,
                           Vector2 origin, float rotation, Color tint);
};
