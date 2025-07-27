#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <algorithm>
#include <random>
#include <functional>



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
    u32 vertexCount { 0 };
    u32 indexCount { 0 };
    u32 quadCount { 0 };
    bool dynamic = false;
    bool isBegin { false };

public:
    void Init(u32 maxQuads, bool dynamic = true) 
    {
        this->maxQuads = maxQuads;
        this->maxVertices = maxQuads * 4;  // 4 vértices por quad
        this->maxIndices = maxQuads * 6;   // 6 índices por quad (2 triângulos)
        this->dynamic = dynamic;

        // Redimensionar vetores
        vertices.resize(maxVertices);
        textcoord.resize(maxVertices);
        colors.resize(maxVertices * 4);    // RGBA para cada vértice
        indices.resize(maxIndices);

        // Inicializar dados default
        for (u32 i = 0; i < maxVertices; i++) 
        {
            colors[i * 4 + 0] = 255;
            colors[i * 4 + 1] = 255;
            colors[i * 4 + 2] = 255;
            colors[i * 4 + 3] = 255;
            textcoord[i] = {0, 0};
            vertices[i] = {0, 0, 0};
        }

        // Gerar índices para quads (cada quad = 2 triângulos)
        for (u32 i = 0; i < maxQuads; i++) 
        {
            int baseVertex = i * 4;
            int baseIndex = i * 6;
            
            // Primeiro triângulo
            indices[baseIndex + 0] = baseVertex + 0;
            indices[baseIndex + 1] = baseVertex + 1;
            indices[baseIndex + 2] = baseVertex + 2;
            
            // Segundo triângulo
            indices[baseIndex + 3] = baseVertex + 0;
            indices[baseIndex + 4] = baseVertex + 2;
            indices[baseIndex + 5] = baseVertex + 3;
        }

        // Criar VAO
        vaoId = rlLoadVertexArray();
        rlEnableVertexArray(vaoId);
 
        vboId[0] = rlLoadVertexBuffer(vertices.data(), maxVertices * sizeof(Vector3), dynamic);
        rlEnableVertexBuffer(vboId[0]);
        rlEnableVertexAttribute(0);
        rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);

 
        vboId[1] = rlLoadVertexBuffer(textcoord.data(), maxVertices * sizeof(Vector2), dynamic);
        rlEnableVertexBuffer(vboId[1]);
        rlEnableVertexAttribute(1);
        rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);

   
        vboId[2] = rlLoadVertexBuffer(colors.data(), maxVertices * 4 * sizeof(u8), dynamic);
        rlEnableVertexBuffer(vboId[2]);
        rlEnableVertexAttribute(2);
        rlSetVertexAttribute(2, 4, RL_UNSIGNED_BYTE, 1, 0, 0);

     
        vboId[3] = rlLoadVertexBufferElement(indices.data(), maxIndices * sizeof(u16), false);

        rlDisableVertexArray();
    }

    void Release() 
    {
        rlUnloadVertexArray(vaoId);
        rlUnloadVertexBuffer(vboId[0]);
        rlUnloadVertexBuffer(vboId[1]);
        rlUnloadVertexBuffer(vboId[2]);
        rlUnloadVertexBuffer(vboId[3]);
    }

    void BeginQuad()
    {
        isBegin = true;
        if (quadCount >= maxQuads) 
        {
            Flush();
        }
    }

    void AddVertex(Vector3 position, Vector2 texCoords, Color color) 
    {
        if (!isBegin)
        {
            LogWarning("BeginQuad() must be called before AddVertex().");
        }
        SetVertex(position.x, position.y, position.z, texCoords.x, texCoords.y, color.r, color.g, color.b, color.a);
    }

    void AddVertex(float x, float y, float z, float u, float v, u8 r, u8 g, u8 b, u8 a) 
    {
        if (!isBegin)
        {
            LogWarning("BeginQuad() must be called before AddVertex().");
        }
        SetVertex(x, y, z, u, v, r, g, b, a);
    }

    void EndQuad() 
    {
        isBegin = false;
        quadCount++;
    }

    void AddQuad(Vector3 position, Vector2 size, Vector2 texCoords[4], Color color) 
    {
        if (quadCount >= maxQuads) 
        {
            Flush();
        }

        u8 r = color.r, g = color.g, b = color.b, a = color.a;
        
        SetVertex(position.x, position.y, position.z, 
                 texCoords[0].x, texCoords[0].y, r, g, b, a);
        
        SetVertex(position.x + size.x, position.y, position.z, 
                 texCoords[1].x, texCoords[1].y, r, g, b, a);
        
        SetVertex(position.x + size.x, position.y + size.y, position.z, 
                 texCoords[2].x, texCoords[2].y, r, g, b, a);
        
        SetVertex(position.x, position.y + size.y, position.z, 
                 texCoords[3].x, texCoords[3].y, r, g, b, a);

        quadCount++;
    }

    void AddQuad(Vector3 v0 , Vector3 v1, Vector3 v2, Vector3 v3, Color color,
                 Vector2 t0, Vector2 t1, Vector2 t2, Vector2 t3) 
    {
        if (quadCount >= maxQuads) 
        {
            Flush();
        }
        u8 r = color.r, g = color.g, b = color.b, a = color.a;

        SetVertex(v0.x, v0.y, v0.z, t0.x, t0.y, r, g, b, a);
        SetVertex(v1.x, v1.y, v1.z, t1.x, t1.y, r, g, b, a);
        SetVertex(v2.x, v2.y, v2.z, t2.x, t2.y, r, g, b, a);
        SetVertex(v3.x, v3.y, v3.z, t3.x, t3.y, r, g, b, a);
        
        quadCount++;
        
    }


    void AddQuad(Vector3 v0 , Vector3 v1, Vector3 v2, Vector3 v3, Color color) 
    {
        if (quadCount >= maxQuads) 
        {
            Flush();
        }
        u8 r = color.r, g = color.g, b = color.b, a = color.a;
        Vector2 defaultTexCoords[4] = {{0,0}, {1,0}, {1,1}, {0,1}};

        SetVertex(v0.x, v0.y, v0.z, defaultTexCoords[0].x, defaultTexCoords[0].y, r, g, b, a);
        SetVertex(v1.x, v1.y, v1.z, defaultTexCoords[1].x, defaultTexCoords[1].y, r, g, b, a);
        SetVertex(v2.x, v2.y, v2.z, defaultTexCoords[2].x, defaultTexCoords[2].y, r, g, b, a);
        SetVertex(v3.x, v3.y, v3.z, defaultTexCoords[3].x, defaultTexCoords[3].y, r, g, b, a);

        quadCount++;
        
    }

 
    void AddQuad(float x, float y, float width, float height, Color color) 
    {
        if (quadCount >= maxQuads) 
        {
            Flush();
        }
        
        Vector2 defaultTexCoords[4] = {{0,0}, {1,0}, {1,1}, {0,1}};
        AddQuad({x, y, 0}, {width, height}, defaultTexCoords, color);
    }

   
    void SetVertex(float x, float y, float z, float u, float v, u8 r, u8 g, u8 b, u8 a) {
        if (vertexCount >= maxVertices) return;

        vertices[vertexCount] = {x, y, z};
        textcoord[vertexCount] = {u, v};
        
        colors[vertexCount * 4 + 0] = r;
        colors[vertexCount * 4 + 1] = g;
        colors[vertexCount * 4 + 2] = b;
        colors[vertexCount * 4 + 3] = a;
        
        vertexCount++;
    }

    
    void Flush() {
        if (quadCount == 0) return;

 
        rlEnableVertexArray(vaoId);
        
        rlUpdateVertexBuffer(vboId[0], vertices.data(), vertexCount * sizeof(Vector3), 0);
        rlUpdateVertexBuffer(vboId[1], textcoord.data(), vertexCount * sizeof(Vector2), 0);
        rlUpdateVertexBuffer(vboId[2], colors.data(), vertexCount * 4 * sizeof(u8), 0);

 
        rlDrawVertexArrayElements(0, quadCount * 6, nullptr);
        
        rlDisableVertexArray();
        
 
        Clear();
    }


    void Render(u32 texture) 
    {
        if (quadCount == 0) return;

        
 
        
        rlActiveTextureSlot(0);
        rlEnableTexture(texture);
        
         
        rlEnableVertexArray(vaoId);
        
        rlUpdateVertexBuffer(vboId[0], vertices.data(), vertexCount * sizeof(Vector3), 0);
        rlUpdateVertexBuffer(vboId[1], textcoord.data(), vertexCount * sizeof(Vector2), 0);
        rlUpdateVertexBuffer(vboId[2], colors.data(),   vertexCount * 4 * sizeof(u8), 0);

        rlDrawVertexArrayElements(0, quadCount * 6, nullptr);
        
       // LogInfo("Rendered %d quads", quadCount);
 

        rlDisableVertexArray();
    }



 
    void Clear() 
    {
        vertexCount = 0;
        quadCount = 0;
    }

 
    u32 GetQuadCount() const { return quadCount; }
    u32 GetVertexCount() const { return vertexCount; }
    bool IsFull() const { return quadCount >= maxQuads; }




    void DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position, float scale, Color tint)
    {
        Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        DrawQuadBillboard(matView, texture, source, position, (Vector2) { scale*fabsf((float)source.width/source.height), scale }, tint);
    }

     void DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position, Vector2 size, float angle, Color tint)
    {
        Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        Vector2 origin = { texture.width * 0.5f, texture.height * 0.5f };
        Vector3 up = { 0.0f, 1.0f, 0.0f };
        DrawQuadBillboard(matView, texture, source, position,up, size, origin, angle, tint);

    }

    void DrawQuadBillboard(Matrix& matView, Texture& texture,  Rectangle source, Vector3 position, Vector2 size, Color tint)
    {
       
        Vector3 up = { 0.0f, 1.0f, 0.0f };

        DrawQuadBillboard(matView, texture, source, position, up, size, Vector2Scale(size, 0.5), 0.0f, tint);
    }


void DrawQuadBillboard(Matrix& matView, Texture& texture,  Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint)
{
    Vector3 right = { matView.m0, matView.m4, matView.m8 };
    right = Vector3Scale(right, size.x);
    up = Vector3Scale(up, size.y);

    // Flip the content of the billboard while maintaining the counterclockwise edge rendering order
    if (size.x < 0.0f)
    {
        source.x += size.x;
        source.width *= -1.0;
        right = Vector3Negate(right);
        origin.x *= -1.0f;
    }
    if (size.y < 0.0f)
    {
        source.y += size.y;
        source.height *= -1.0;
        up = Vector3Negate(up);
        origin.y *= -1.0f;
    }

    Vector3 forward;
    if (rotation != 0.0) forward = Vector3CrossProduct(right, up);

    Vector3 origin3D = Vector3Add(Vector3Scale(Vector3Normalize(right), origin.x), Vector3Scale(Vector3Normalize(up), origin.y));

    Vector3 points[4];
    points[0] = Vector3Zero();
    points[1] = right;
    points[2] = Vector3Add(up, right);
    points[3] = up;

    for (int i = 0; i < 4; i++)
    {
        points[i] = Vector3Subtract(points[i], origin3D);
        if (rotation != 0.0) points[i] = Vector3RotateByAxisAngle(points[i], forward, rotation*DEG2RAD);
        points[i] = Vector3Add(points[i], position);
    }

    BeginQuad();

    Vector2 texcoords[4];
    texcoords[0] = (Vector2){ (float)source.x/texture.width, (float)(source.y + source.height)/texture.height };
    texcoords[1] = (Vector2){ (float)(source.x + source.width)/texture.width, (float)(source.y + source.height)/texture.height };
    texcoords[2] = (Vector2){ (float)(source.x + source.width)/texture.width, (float)source.y/texture.height };
    texcoords[3] = (Vector2){ (float)source.x/texture.width, (float)source.y/texture.height };

        for (int i = 0; i < 4; i++)
        {

            AddVertex(points[i].x, points[i].y, points[i].z, texcoords[i].x, texcoords[i].y, tint.r, tint.g, tint.b, tint.a);

        }

    EndQuad();
}
};

 

 /*
void DrawManySprites() {
    for (auto& sprite : sprites) {
        batch.AddQuad(sprite.x, sprite.y, sprite.w, sprite.h, sprite.color);
        
        //  mudar de textura 
        if (sprite.needsNewTexture) {
            batch.Flush();
            BindTexture(sprite.texture);
        }
    }
    batch.Flush();  
}


void TestBillboard() {
    Vector3 pos = {0, 0, -5};
    Vector2 size = {2, 2};
    

    Vector3 right = {1, 0, 0};  // X axis
    Vector3 up = {0, 1, 0};     // Y axis
    
    right = Vector3Scale(right, size.x);
    up = Vector3Scale(up, size.y);
    
    Vector3 points[4] = {
        {pos.x - 1, pos.y - 1, pos.z},  // bottom-left
        {pos.x + 1, pos.y - 1, pos.z},  // bottom-right
        {pos.x + 1, pos.y + 1, pos.z},  // top-right
        {pos.x - 1, pos.y + 1, pos.z}   // top-left
    };
    
    batch.BeginQuad();
    batch.AddVertex(points[0], {0,1}, WHITE);
    batch.AddVertex(points[1], {1,1}, WHITE);
    batch.AddVertex(points[2], {1,0}, WHITE);
    batch.AddVertex(points[3], {0,0}, WHITE);
    batch.EndQuad();
}
*/