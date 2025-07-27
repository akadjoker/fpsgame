#pragma once
#include "Config.hpp"
#include "collision.hpp"
// #include <raylib.h>
// #include <raymath.h>
// #include <vector>
// #include <string>

class Model3D;
class Scene;

class MatrixStack 
{
private:
    std::vector<Matrix> stack;

public:
    // Constructor - starts with identity matrix
    MatrixStack()
    {
        stack.reserve(32); // Reserve space for typical usage
        LoadIdentity();
    }

 
    void LoadIdentity()
    {
        if (stack.empty())
        {
            stack.push_back(MatrixIdentity());
        }
        else
        {
            stack.back() = MatrixIdentity();
        }
    }

    // glPushMatrix() equivalent - duplicate current matrix
    void PushMatrix()
    {
        if (stack.empty())
        {
            stack.push_back(MatrixIdentity());
        }
        else
        {
            stack.push_back(stack.back()); // Copy current matrix
        }
    }

 
    void PopMatrix()
    {
        if (stack.size() > 1)
        { // Keep at least one matrix
            stack.pop_back();
        }
    }

    // Get current matrix (like getting GL_MODELVIEW_MATRIX)
    const Matrix& Top() const
    {
        static Matrix identity = MatrixIdentity();
        return stack.empty() ? identity : stack.back();
    }

    // Reset stack to single identity matrix
    void Reset()
    {
        stack.clear();
        LoadIdentity();
    }

    // ===== TRANSFORM OPERATIONS (modify current matrix) =====

    // glTranslatef() equivalent
    void Translate(float x, float y, float z)
    {
        if (!stack.empty())
        {
            Matrix translation = MatrixTranslate(x, y, z);
            stack.back() = MatrixMultiply(stack.back(), translation);
        }
    }

    void Translate(const Vector3& translation)
    {
        Translate(translation.x, translation.y, translation.z);
    }

    // glRotatef() equivalent - rotate around axis
    void Rotate(float angle, float x, float y, float z)
    {
        if (!stack.empty())
        {
            Matrix rotation = MatrixRotate({ x, y, z }, angle * DEG2RAD);
            stack.back() = MatrixMultiply(stack.back(), rotation);
        }
    }

    void Rotate(float angle, const Vector3& axis)
    {
        Rotate(angle, axis.x, axis.y, axis.z);
    }

    // Rotate using quaternion
    void Rotate(const Quaternion& quat)
    {
        if (!stack.empty())
        {
            Matrix rotation = QuaternionToMatrix(quat);
            stack.back() = MatrixMultiply(stack.back(), rotation);
        }
    }

    // Rotate using Euler angles (degrees)
    void RotateEuler(float pitch, float yaw, float roll)
    {
        Quaternion q =
            QuaternionFromEuler(pitch * DEG2RAD, yaw * DEG2RAD, roll * DEG2RAD);
        Rotate(q);
    }

    // Individual axis rotations (like glRotatef)
    void RotateX(float angle) { Rotate(angle, 1.0f, 0.0f, 0.0f); }
    void RotateY(float angle) { Rotate(angle, 0.0f, 1.0f, 0.0f); }
    void RotateZ(float angle) { Rotate(angle, 0.0f, 0.0f, 1.0f); }

    // glScalef() equivalent
    void Scale(float x, float y, float z)
    {
        if (!stack.empty())
        {
            Matrix scaling = MatrixScale(x, y, z);
            stack.back() = MatrixMultiply(stack.back(), scaling);
        }
    }

    void Scale(const Vector3& scale) { Scale(scale.x, scale.y, scale.z); }

    void Scale(float uniform) { Scale(uniform, uniform, uniform); }

 
    void MultMatrix(const Matrix& matrix)
    {
        if (!stack.empty())
        {
            stack.back() = MatrixMultiply(stack.back(), matrix);
        }
    }
 
    void LoadMatrix(const Matrix& matrix)
    {
        if (stack.empty())
        {
            stack.push_back(matrix);
        }
        else
        {
            stack.back() = matrix;
        }
    }

   
    size_t Depth() const { return stack.size(); }
 
    bool Empty() const { return stack.empty(); }

 
    Vector3 TransformPoint(const Vector3& point) const
    {
        return Vector3Transform(point, Top());
    }
 
    Vector3 TransformDirection(const Vector3& direction) const
    {
        // Extract rotation part and apply to direction
        Matrix mat = Top();
        Vector3 result = {
            mat.m0 * direction.x + mat.m4 * direction.y + mat.m8 * direction.z,
            mat.m1 * direction.x + mat.m5 * direction.y + mat.m9 * direction.z,
            mat.m2 * direction.x + mat.m6 * direction.y + mat.m10 * direction.z
        };
        return result;
    }

 
    Vector3 GetRight() const
    {
        Matrix mat = Top();
        return { mat.m0, mat.m1, mat.m2 };
    }

    Vector3 GetUp() const
    {
        Matrix mat = Top();
        return { mat.m4, mat.m5, mat.m6 };
    }

    Vector3 GetForward() const
    {
        Matrix mat = Top();
        return { -mat.m8, -mat.m9, -mat.m10 }; // -Z is forward
    }

    Vector3 GetPosition() const
    {
        Matrix mat = Top();
        return { mat.m12, mat.m13, mat.m14 };
    }
 
    void ApplyMD3Tag(const Vector3& origin, const Vector3 axis[3],
                     bool convertCoords = true)
    {
        if (convertCoords)
        {
            // Convert MD3 coordinates (Z-up) to Raylib (Y-up)
            Vector3 convertedOrigin = { origin.x, origin.z, -origin.y };

            Matrix tagMatrix = {
                axis[0].x, axis[0].z, -axis[0].y, convertedOrigin.x,
                axis[1].x, axis[1].z, -axis[1].y, convertedOrigin.y,
                axis[2].x, axis[2].z, -axis[2].y, convertedOrigin.z,
                0.0f,      0.0f,      0.0f,       1.0f
            };

            MultMatrix(tagMatrix);
        }
        else
        {
            Matrix tagMatrix = { axis[0].x, axis[0].y, axis[0].z, origin.x,
                                 axis[1].x, axis[1].y, axis[1].z, origin.y,
                                 axis[2].x, axis[2].y, axis[2].z, origin.z,
                                 0.0f,      0.0f,      0.0f,      1.0f };

            MultMatrix(tagMatrix);
        }
    }
};

class Node3D {
public:
    std::string name{ "Node3D" };
       int tag = 0;
    void* userData = nullptr;

    // Transformação local (relativa ao pai)
    Vector3 localPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 localScale = { 1.0f, 1.0f, 1.0f };

    // Transformação mundial (absoluta)
    Vector3 worldPosition = { 0.0f, 0.0f, 0.0f };
    Quaternion worldRotation = { 0.0f, 0.0f, 0.0f };
    Vector3 worldScale = { 1.0f, 1.0f, 1.0f };

    // Hierarquia
    Node3D* parent = nullptr;

    Matrix local;


    Node3D();
    Node3D(Vector3 position);
    Node3D(Vector3 position, Vector3 rotation);

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
    void Rotate(Vector3 rotation);
    void Scale(Vector3 scale);

    virtual void Render() {}
    virtual void Update(float dt) {}

    void SetWorldPosition(Vector3 position);
    void SetWorldRotation(Vector3 rotation);
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
     u32 ID = 0;
     friend class Scene;
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


class Scene
{
    std::vector<Model3D*> nodes;
    std::vector<Model3D*> toRemove;


public:



    void AddToRemove(Model3D* node) { toRemove.push_back(node); }

    Model3D* AddNode(Model* node);
    Model3D* AddNode(Model* model,Vector3 position);
    Model3D* AddNode(Model* model,Vector3 position,Vector3 scale);
    void RemoveNode(Model3D* node);
    void Update(float dt);
    void Render();
    void Clear();



    Model3D* GetNode(u32 index) { return nodes[index]; }

    bool collide(const BoundingBox& area, PickData* data) ;
    bool collide(const Vector3& point, float radius, PickData *data) ;
    bool collide(const Ray& ray, float maxDistance, PickData* data) ;

    std::vector<const Triangle*> collectTriangles(const BoundingBox& area) const;

};