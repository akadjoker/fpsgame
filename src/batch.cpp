 
#include "batch.hpp"

void Batch::Init(u32 maxQuads, bool dynamic) 
{
    this->maxQuads = maxQuads;
    this->maxVertices = maxQuads * 4;
    this->maxIndices = maxQuads * 6;
    this->dynamic = dynamic;

    vertices.resize(maxVertices);
    textcoord.resize(maxVertices);
    colors.resize(maxVertices * 4);
    indices.resize(maxIndices);

    for (u32 i = 0; i < maxVertices; i++) 
    {
        colors[i * 4 + 0] = 255;
        colors[i * 4 + 1] = 255;
        colors[i * 4 + 2] = 255;
        colors[i * 4 + 3] = 255;
        textcoord[i] = {0, 0};
        vertices[i] = {0, 0, 0};
    }

    for (u32 i = 0; i < maxQuads; i++) 
    {
        int baseVertex = i * 4;
        int baseIndex = i * 6;

        indices[baseIndex + 0] = baseVertex + 0;
        indices[baseIndex + 1] = baseVertex + 1;
        indices[baseIndex + 2] = baseVertex + 2;
        indices[baseIndex + 3] = baseVertex + 0;
        indices[baseIndex + 4] = baseVertex + 2;
        indices[baseIndex + 5] = baseVertex + 3;
    }

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

void Batch::Release() 
{
    rlUnloadVertexArray(vaoId);
    rlUnloadVertexBuffer(vboId[0]);
    rlUnloadVertexBuffer(vboId[1]);
    rlUnloadVertexBuffer(vboId[2]);
    rlUnloadVertexBuffer(vboId[3]);
}

void Batch::BeginQuad()
{
    isBegin = true;
    if (quadCount >= maxQuads) Flush();
}

void Batch::AddVertex(Vector3 position, Vector2 texCoords, Color color) 
{
    AddVertex(position.x, position.y, position.z, texCoords.x, texCoords.y, color.r, color.g, color.b, color.a);
}

void Batch::AddVertex(float x, float y, float z, float u, float v, u8 r, u8 g, u8 b, u8 a) 
{
    if (!isBegin) LogWarning("BeginQuad() must be called before AddVertex().");
    SetVertex(x, y, z, u, v, r, g, b, a);
}

void Batch::EndQuad() 
{
    isBegin = false;
    quadCount++;
}

void Batch::AddQuad(Vector3 position, Vector2 size, Vector2 texCoords[4], Color color) 
{
    if (quadCount >= maxQuads) Flush();

    u8 r = color.r, g = color.g, b = color.b, a = color.a;

    SetVertex(position.x, position.y, position.z, texCoords[0].x, texCoords[0].y, r, g, b, a);
    SetVertex(position.x + size.x, position.y, position.z, texCoords[1].x, texCoords[1].y, r, g, b, a);
    SetVertex(position.x + size.x, position.y + size.y, position.z, texCoords[2].x, texCoords[2].y, r, g, b, a);
    SetVertex(position.x, position.y + size.y, position.z, texCoords[3].x, texCoords[3].y, r, g, b, a);

    quadCount++;
}

void Batch::AddQuad(Vector3 v0 , Vector3 v1, Vector3 v2, Vector3 v3, Color color, Vector2 t0, Vector2 t1, Vector2 t2, Vector2 t3) 
{
    if (quadCount >= maxQuads) Flush();
    u8 r = color.r, g = color.g, b = color.b, a = color.a;

    SetVertex(v0.x, v0.y, v0.z, t0.x, t0.y, r, g, b, a);
    SetVertex(v1.x, v1.y, v1.z, t1.x, t1.y, r, g, b, a);
    SetVertex(v2.x, v2.y, v2.z, t2.x, t2.y, r, g, b, a);
    SetVertex(v3.x, v3.y, v3.z, t3.x, t3.y, r, g, b, a);
    quadCount++;
}

void Batch::AddQuad(Vector3 v0 , Vector3 v1, Vector3 v2, Vector3 v3, Color color) 
{
    Vector2 defaultTexCoords[4] = {{0,0}, {1,0}, {1,1}, {0,1}};
    AddQuad(v0, v1, v2, v3, color, defaultTexCoords[0], defaultTexCoords[1], defaultTexCoords[2], defaultTexCoords[3]);
}

void Batch::AddQuad(float x, float y, float width, float height, Color color) 
{
    Vector2 defaultTexCoords[4] = {{0,0}, {1,0}, {1,1}, {0,1}};
    AddQuad({x, y, 0}, {width, height}, defaultTexCoords, color);
}

void Batch::SetVertex(float x, float y, float z, float u, float v, u8 r, u8 g, u8 b, u8 a) 
{
    if (vertexCount >= maxVertices) return;

    vertices[vertexCount] = {x, y, z};
    textcoord[vertexCount] = {u, v};
    colors[vertexCount * 4 + 0] = r;
    colors[vertexCount * 4 + 1] = g;
    colors[vertexCount * 4 + 2] = b;
    colors[vertexCount * 4 + 3] = a;
    vertexCount++;
}

void Batch::Flush() 
{
    if (quadCount == 0) return;

    rlEnableVertexArray(vaoId);
    rlUpdateVertexBuffer(vboId[0], vertices.data(), vertexCount * sizeof(Vector3), 0);
    rlUpdateVertexBuffer(vboId[1], textcoord.data(), vertexCount * sizeof(Vector2), 0);
    rlUpdateVertexBuffer(vboId[2], colors.data(), vertexCount * 4 * sizeof(u8), 0);
    rlDrawVertexArrayElements(0, quadCount * 6, nullptr);
    rlDisableVertexArray();

    Clear();
}

void Batch::Render(u32 texture) 
{
    if (quadCount == 0) return;

    rlActiveTextureSlot(0);
    rlEnableTexture(texture);

    rlEnableVertexArray(vaoId);
    rlUpdateVertexBuffer(vboId[0], vertices.data(), vertexCount * sizeof(Vector3), 0);
    rlUpdateVertexBuffer(vboId[1], textcoord.data(), vertexCount * sizeof(Vector2), 0);
    rlUpdateVertexBuffer(vboId[2], colors.data(), vertexCount * 4 * sizeof(u8), 0);

    rlDrawVertexArrayElements(0, quadCount * 6, nullptr);
    rlDisableVertexArray();
}

void Batch::Clear() 
{
    vertexCount = 0;
    quadCount = 0;
}

u32 Batch::GetQuadCount() const { return quadCount; }
u32 Batch::GetVertexCount() const { return vertexCount; }
bool Batch::IsFull() const { return quadCount >= maxQuads; }

void Batch::DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position, float scale, Color tint)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    DrawQuadBillboard(matView, texture, source, position, { scale * fabsf((float)source.width / source.height), scale }, tint);
}

void Batch::DrawQuadBillboard(Matrix& matView, Texture& texture, Vector3 position, Vector2 size, float angle, Color tint)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Vector2 origin = { texture.width * 0.5f, texture.height * 0.5f };
    Vector3 up = { 0.0f, 1.0f, 0.0f };
    DrawQuadBillboard(matView, texture, source, position, up, size, origin, angle, tint);
}

void Batch::DrawQuadBillboard(Matrix& matView, Texture& texture, Rectangle source, Vector3 position, Vector2 size, Color tint)
{
    Vector3 up = { 0.0f, 1.0f, 0.0f };
    DrawQuadBillboard(matView, texture, source, position, up, size, Vector2Scale(size, 0.5), 0.0f, tint);
}

void Batch::DrawQuadBillboard(Matrix& matView, Texture& texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint)
{
    Vector3 right = { matView.m0, matView.m4, matView.m8 };
    right = Vector3Scale(right, size.x);
    up = Vector3Scale(up, size.y);

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
    if (rotation != 0.0f) forward = Vector3CrossProduct(right, up);

    Vector3 origin3D = Vector3Add(Vector3Scale(Vector3Normalize(right), origin.x), Vector3Scale(Vector3Normalize(up), origin.y));

    Vector3 points[4] = {
        Vector3Subtract(Vector3Zero(), origin3D),
        Vector3Subtract(right, origin3D),
        Vector3Subtract(Vector3Add(up, right), origin3D),
        Vector3Subtract(up, origin3D)
    };

    for (int i = 0; i < 4; i++)
    {
        if (rotation != 0.0f) points[i] = Vector3RotateByAxisAngle(points[i], forward, rotation * DEG2RAD);
        points[i] = Vector3Add(points[i], position);
    }

    BeginQuad();

    Vector2 texcoords[4] = {
        { source.x / texture.width, (source.y + source.height) / texture.height },
        { (source.x + source.width) / texture.width, (source.y + source.height) / texture.height },
        { (source.x + source.width) / texture.width, source.y / texture.height },
        { source.x / texture.width, source.y / texture.height }
    };

    for (int i = 0; i < 4; i++)
    {
        AddVertex(points[i].x, points[i].y, points[i].z, texcoords[i].x, texcoords[i].y, tint.r, tint.g, tint.b, tint.a);
    }

    EndQuad();
}
