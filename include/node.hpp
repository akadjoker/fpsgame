#pragma once
#include "Config.hpp"
#include "collision.hpp"
// #include <raylib.h>
// #include <raymath.h>
// #include <vector>
// #include <string>

class Model3D;
class Scene;
#pragma once

#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>
#include <cstdint>

using u32 = uint32_t;

class Node3D 
{
public:
    std::string name { "Node3D" };
    int tag = 0;
    void* userData = nullptr;

    // Transformação local
    Vector3 localPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 localScale = { 1.0f, 1.0f, 1.0f };

    // Transformação mundial
    Vector3 worldPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion worldRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 worldScale = { 1.0f, 1.0f, 1.0f };

    // Hierarquia
    Node3D* parent = nullptr;

    Node3D();
    Node3D(Vector3 position);
    Node3D(Vector3 position, Vector3 rotationEuler);
    virtual ~Node3D();

    void AddChild(Node3D* child);
    void RemoveChild(Node3D* child);
    void SetParent(Node3D* newParent);

    void SetLocalMatrix(Matrix matrix);
    void SetLocalPosition(Vector3 position);
    void SetLocalRotation(Quaternion rotation);
    void SetLocalRotationEuler(Vector3 eulerAngles);
    void SetLocalScale(Vector3 scale);
    void Translate(Vector3 translation);
    void Rotate(Vector3 eulerRotation);
    void Scale(Vector3 scale);

    virtual void Render() {}
    virtual void Update(float dt);

    void SetWorldPosition(Vector3 position);
    void SetWorldRotation(Quaternion rotation);

    Vector3 GetWorldPosition() const { return worldPosition; }
    Quaternion GetWorldRotation() const { return worldRotation; }
    Vector3 GetWorldScale() const { return worldScale; }

    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;

    Matrix GetLocalMatrix() const;
    Matrix GetWorldMatrix() const;
    Matrix GetInverseWorldMatrix() const;

    u32 GetID() const { return ID; }

private:
    std::vector<Node3D*> children;
    friend class Scene;
    Matrix local;
    u32 ID = 0;

    void UpdateWorldTransform();
    void UpdateChildrenWorldTransform();
};


struct PickData
{
    Vector3 intersectionPoint;
    Triangle intersectionTriangle;
    s32 triangleIndex;
    s32 triangleHits;
    bool collide;
    Model3D* node;
};

class Model3D : public Node3D 
{
    Model *model{nullptr};
    Color color;
    BoundingBox bounds;
    BoundingBox world;
    bool m_visible = true;
    mutable std::vector<Triangle> cachedTriangles;  // Cache dos triângulos
    mutable bool trianglesCacheValid = false;

public:
    Model3D(Model* model);
    ~Model3D();
    void Render();
    void SetTexture(u32 index , Texture2D texture);

    void SetVisible(bool visible) { m_visible = visible; }
    
    bool collide(const BoundingBox& area, PickData* data) ;
    bool collide(const Vector3& point, float radius, PickData *data) ;
    bool collide(const Ray& ray, float maxDistance, PickData* data) ;

      //std::vector<const Triangle*> triangles;

    bool collectTriangles(const BoundingBox& area, std::vector<const Triangle*>& out) const;
    bool collectTriangles(const Ray& ray, std::vector<const Triangle*>& out) const;
    bool collectTriangles(const Vector3& point, float radius, std::vector<const Triangle*>& out) const;

    std::vector<Triangle> GetTriangles(bool transform = false);

    void invalidateTriangleCache() {        trianglesCacheValid = false;}
    void buildTriangleCache() const;
};
