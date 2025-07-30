 
#include "node.hpp"
 


static u32 s_nextID = 1;

Node3D::Node3D()
{
    ID = s_nextID++;
    local = MatrixIdentity();
}

Node3D::Node3D(Vector3 position)
{
    ID = s_nextID++;
    localPosition = position;
    local = GetLocalMatrix();
}

Node3D::Node3D(Vector3 position, Vector3 rotationEuler)
{
    ID = s_nextID++;
    localPosition = position;
    localRotation = QuaternionFromEuler(rotationEuler.x * DEG2RAD, rotationEuler.y * DEG2RAD, rotationEuler.z * DEG2RAD);
    local = GetLocalMatrix();
}

Node3D::~Node3D()
{
    for (auto* child : children)
    {
        child->parent = nullptr;
    }
    children.clear();
}

void Node3D::AddChild(Node3D* child)
{
    if (!child) return;
    if (child->parent == this) return;

    child->SetParent(this);
}

void Node3D::RemoveChild(Node3D* child)
{
    if (!child) return;
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    child->parent = nullptr;
}

void Node3D::SetParent(Node3D* newParent)
{
    if (parent)
    {
        parent->RemoveChild(this);
    }
    parent = newParent;
    if (newParent)
    {
        newParent->children.push_back(this);
    }
    UpdateWorldTransform();
}

void Node3D::SetLocalMatrix(Matrix matrix)
{
    local = matrix;

    // Decompor matriz em posição, rotação e escala
    localPosition = { matrix.m12, matrix.m13, matrix.m14 };
    localScale = {
        Vector3Length({ matrix.m0, matrix.m1, matrix.m2 }),
        Vector3Length({ matrix.m4, matrix.m5, matrix.m6 }),
        Vector3Length({ matrix.m8, matrix.m9, matrix.m10 })
    };

    Matrix rot = matrix;
    rot.m12 = rot.m13 = rot.m14 = 0;
    rot = MatrixMultiply(rot, MatrixScale(1.0f / localScale.x, 1.0f / localScale.y, 1.0f / localScale.z));
    localRotation = QuaternionFromMatrix(rot);

    UpdateWorldTransform();
}

void Node3D::SetLocalPosition(Vector3 position)
{
    localPosition = position;
    UpdateWorldTransform();
}

void Node3D::SetLocalRotation(Quaternion rotation)
{
    localRotation = rotation;
    UpdateWorldTransform();
}

void Node3D::SetLocalRotationEuler(Vector3 eulerAngles)
{
    localRotation = QuaternionFromEuler(eulerAngles.x * DEG2RAD, eulerAngles.y * DEG2RAD, eulerAngles.z * DEG2RAD);
    UpdateWorldTransform();
}

void Node3D::SetLocalScale(Vector3 scale)
{
    localScale = scale;
    UpdateWorldTransform();
}

void Node3D::Translate(Vector3 translation)
{
    localPosition = Vector3Add(localPosition, translation);
    UpdateWorldTransform();
}

void Node3D::Rotate(Vector3 eulerRotation)
{
    Quaternion delta = QuaternionFromEuler(eulerRotation.x * DEG2RAD, eulerRotation.y * DEG2RAD, eulerRotation.z * DEG2RAD);
    localRotation = QuaternionMultiply(localRotation, delta);
    UpdateWorldTransform();
}

void Node3D::Scale(Vector3 scale)
{
    localScale.x *= scale.x;
    localScale.y *= scale.y;
    localScale.z *= scale.z;
    UpdateWorldTransform();
}

void Node3D::SetWorldPosition(Vector3 position)
{
    if (parent)
    {
        Matrix invParent = parent->GetInverseWorldMatrix();
        localPosition = Vector3Transform(position, invParent);
    }
    else
    {
        localPosition = position;
    }
    UpdateWorldTransform();
}

void Node3D::SetWorldRotation(Quaternion rotation)
{
    if (parent)
    {
        Quaternion invParent = QuaternionInvert(parent->worldRotation);
        localRotation = QuaternionMultiply(invParent, rotation);
    }
    else
    {
        localRotation = rotation;
    }
    UpdateWorldTransform();
}

Vector3 Node3D::GetForward() const
{
    return Vector3Transform({ 0, 0, -1 }, GetWorldMatrix());
}

Vector3 Node3D::GetRight() const
{
    return Vector3Transform({ 1, 0, 0 }, GetWorldMatrix());
}

Vector3 Node3D::GetUp() const
{
    return Vector3Transform({ 0, 1, 0 }, GetWorldMatrix());
}

Matrix Node3D::GetLocalMatrix() const
{
    return MatrixMultiply(
        MatrixMultiply(MatrixScale(localScale.x, localScale.y, localScale.z), QuaternionToMatrix(localRotation)),
        MatrixTranslate(localPosition.x, localPosition.y, localPosition.z));
}

Matrix Node3D::GetWorldMatrix() const
{
    if (parent)
    {
        return MatrixMultiply(GetLocalMatrix(), parent->GetWorldMatrix());
    }
    return GetLocalMatrix();
}

Matrix Node3D::GetInverseWorldMatrix() const
{
    return MatrixInvert(GetWorldMatrix());
}

void Node3D::Update(float dt)
{
    // Este Update é chamado por cena ou manualmente
    UpdateWorldTransform();
    for (auto* child : children)
    {
        child->Update(dt);
    }
}

void Node3D::UpdateWorldTransform()
{
    Matrix world = GetWorldMatrix();

    worldPosition = { world.m12, world.m13, world.m14 };

    worldScale = {
        Vector3Length({ world.m0, world.m1, world.m2 }),
        Vector3Length({ world.m4, world.m5, world.m6 }),
        Vector3Length({ world.m8, world.m9, world.m10 })
    };

    Matrix rot = world;
    rot.m12 = rot.m13 = rot.m14 = 0;
    rot = MatrixMultiply(rot, MatrixScale(1.0f / worldScale.x, 1.0f / worldScale.y, 1.0f / worldScale.z));
    worldRotation = QuaternionFromMatrix(rot);

    UpdateChildrenWorldTransform();
}

void Node3D::UpdateChildrenWorldTransform()
{
    for (auto* child : children)
    {
        child->UpdateWorldTransform();
    }
}


Model3D::Model3D(Model* model) : model(model)
{
    color = WHITE;
    bounds = GetMeshBoundingBox(model->meshes[0]); 
}

Model3D::~Model3D() 
{
    this->model=nullptr;
}


void Model3D::Render()
{
    if (!m_visible) return;
    this->model->transform = this->GetWorldMatrix();

    world.min = Vector3Transform(this->bounds.min, this->model->transform);
    world.max = Vector3Transform(this->bounds.max, this->model->transform);
    

    for (int i = 0; i < model->meshCount; i++)
    {
        Color c = model->materials[model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)(((int)c.r*(int)this->color.r)/255);
        colorTint.g = (unsigned char)(((int)c.g*(int)this->color.g)/255);
        colorTint.b = (unsigned char)(((int)c.b*(int)this->color.b)/255);
        colorTint.a = (unsigned char)(((int)c.a*(int)this->color.a)/255);

        model->materials[model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        DrawMesh(model->meshes[i], model->materials[model->meshMaterial[i]], model->transform);
        model->materials[model->meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
   // DrawBoundingBox(world, RED);
}

void Model3D::SetTexture(u32 index , Texture2D texture)
{
    if (!model) return;
    if (index >= (u32)model->materialCount) return; //model->materialCount
    model->materials[index].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}


bool CheckCollisionPointBox(const Vector3& point, const BoundingBox& box)
{
    return (point.x >= box.min.x && point.x <= box.max.x &&
            point.y >= box.min.y && point.y <= box.max.y &&
            point.z >= box.min.z && point.z <= box.max.z);
}

bool Model3D::collide(const Ray& ray, float maxDistance, PickData *data) 
{
    if (!m_visible) return false;
    PickData pickData;
    pickData.collide = false;
    pickData.node=nullptr;
    
    Matrix worldMat = GetWorldMatrix();
    RayCollision collision = GetRayCollisionBox(ray, world);

    
    if (collision.hit && collision.distance <= maxDistance)
    {
        float closestDistance = maxDistance;
        bool foundHit = false;
        
        for (int i = 0; i < model->meshCount; i++)
        {
            Mesh mesh = model->meshes[i];
            
            for (int j = 0; j < mesh.triangleCount * 3; j += 3)
            {
                int i0 = mesh.indices[j + 0];
                int i1 = mesh.indices[j + 1];
                int i2 = mesh.indices[j + 2];
                
                Triangle triangle;
                triangle.pointA = {
                    mesh.vertices[i0 * 3 + 0],
                    mesh.vertices[i0 * 3 + 1],
                    mesh.vertices[i0 * 3 + 2]
                };
                triangle.pointB = {
                    mesh.vertices[i1 * 3 + 0],
                    mesh.vertices[i1 * 3 + 1],
                    mesh.vertices[i1 * 3 + 2]
                };
                triangle.pointC = {
                    mesh.vertices[i2 * 3 + 0],
                    mesh.vertices[i2 * 3 + 1],
                    mesh.vertices[i2 * 3 + 2]
                };
                
                // Transform triangle vertices to world space
                triangle.pointA = Vector3Transform(triangle.pointA, worldMat);
                triangle.pointB = Vector3Transform(triangle.pointB, worldMat);
                triangle.pointC = Vector3Transform(triangle.pointC, worldMat);
                triangle.updateBounds();
                
                // Test ray-triangle intersection
                RayCollision triangleCollision = GetRayCollisionTriangle(ray, triangle.pointA, triangle.pointB, triangle.pointC);
                
                if (triangleCollision.hit && triangleCollision.distance <= closestDistance)
                {
                    closestDistance = triangleCollision.distance;
                    foundHit = true;
                    
                
                    pickData.intersectionPoint = triangleCollision.point;
                    pickData.intersectionTriangle = triangle;
                    pickData.triangleIndex = j / 3; // Convert back to triangle index
                    pickData.triangleHits++;
                    pickData.node = this;
                }
            }
        }
        
        if (foundHit && data != nullptr)
        {
            data->collide = true;
            data->intersectionPoint = pickData.intersectionPoint;
            data->intersectionTriangle = pickData.intersectionTriangle;
            data->triangleIndex = pickData.triangleIndex;
            data->triangleHits = pickData.triangleHits;
            data->node = pickData.node;
        }
        
        return foundHit;
    }
    
    return false;
}

 void Model3D::buildTriangleCache() const
 {
        cachedTriangles.clear();
        Matrix worldMat = GetWorldMatrix();
        
        for (int i = 0; i < model->meshCount; i++)
        {
            Mesh mesh = model->meshes[i];
            
            for (int j = 0; j < mesh.triangleCount * 3; j += 3)
            {
                int i0 = mesh.indices[j + 0];
                int i1 = mesh.indices[j + 1];
                int i2 = mesh.indices[j + 2];
                
                Triangle triangle;
                triangle.pointA = {
                    mesh.vertices[i0 * 3 + 0],
                    mesh.vertices[i0 * 3 + 1],
                    mesh.vertices[i0 * 3 + 2]
                };
                triangle.pointB = {
                    mesh.vertices[i1 * 3 + 0],
                    mesh.vertices[i1 * 3 + 1],
                    mesh.vertices[i1 * 3 + 2]
                };
                triangle.pointC = {
                    mesh.vertices[i2 * 3 + 0],
                    mesh.vertices[i2 * 3 + 1],
                    mesh.vertices[i2 * 3 + 2]
                };
                
                // Transform triangle vertices to world space
                triangle.pointA = Vector3Transform(triangle.pointA, worldMat);
                triangle.pointB = Vector3Transform(triangle.pointB, worldMat);
                triangle.pointC = Vector3Transform(triangle.pointC, worldMat);
                triangle.updateBounds();
                
                cachedTriangles.push_back(triangle);   
            }
        }
 }

bool Model3D::collectTriangles(const BoundingBox& area, std::vector<const Triangle*>& out) const
{
     if (!m_visible) return false;
     if (!CheckCollisionBoxes(world, area)) return false;
    

        if (!trianglesCacheValid) 
        {
            buildTriangleCache();
            trianglesCacheValid = true;
        }
        
        
        for (const Triangle& triangle : cachedTriangles) 
        {
            if (CheckCollisionBoxes(triangle.bounds, area)) 
            {
                out.push_back(&triangle);
            }
        }
        
        return !out.empty();
}

bool Model3D::collide(const BoundingBox& area, PickData* data)
{
    if (!m_visible) return false;
    // First check if model's bounding box intersects with the area
    if (!CheckCollisionBoxes(world, area))
        return false;
    
    PickData pickData;
    bool foundHit = false;
    
    for (int i = 0; i < model->meshCount; i++)
    {
        Mesh mesh = model->meshes[i];
        
        for (int j = 0; j < mesh.triangleCount * 3; j += 3)
        {
            int i0 = mesh.indices[j + 0];
            int i1 = mesh.indices[j + 1];
            int i2 = mesh.indices[j + 2];
            
            Triangle triangle;
            triangle.pointA = {
                mesh.vertices[i0 * 3 + 0],
                mesh.vertices[i0 * 3 + 1],
                mesh.vertices[i0 * 3 + 2]
            };
            triangle.pointB = {
                mesh.vertices[i1 * 3 + 0],
                mesh.vertices[i1 * 3 + 1],
                mesh.vertices[i1 * 3 + 2]
            };
            triangle.pointC = {
                mesh.vertices[i2 * 3 + 0],
                mesh.vertices[i2 * 3 + 1],
                mesh.vertices[i2 * 3 + 2]
            };
            
            // Transform triangle vertices to world space
            triangle.pointA = Vector3Transform(triangle.pointA, this->GetWorldMatrix());
            triangle.pointB = Vector3Transform(triangle.pointB, this->GetWorldMatrix());
            triangle.pointC = Vector3Transform(triangle.pointC, this->GetWorldMatrix());
            triangle.updateBounds();
            
            
            // Check if any vertex of the triangle is inside the bounding box
            // or if the triangle intersects with the bounding box
            bool vertexInside = CheckCollisionPointBox(triangle.pointA, area) ||
                               CheckCollisionPointBox(triangle.pointB, area) ||
                               CheckCollisionPointBox(triangle.pointC, area);
            
            if (vertexInside || CheckCollisionBoxes(triangle.bounds, area))
            {
                foundHit = true;
                pickData.triangleHits++;
                
                if (data != nullptr)
                {
                    // Store first hit or closest to center of area
                    if (!pickData.collide)
                    {
                        // Vector3 areaCenter = {
                        //     (area.min.x + area.max.x) * 0.5f,
                        //     (area.min.y + area.max.y) * 0.5f,
                        //     (area.min.z + area.max.z) * 0.5f
                        // };
                        
                        Vector3 triangleCenter = {
                            (triangle.pointA.x + triangle.pointB.x + triangle.pointC.x) / 3.0f,
                            (triangle.pointA.y + triangle.pointB.y + triangle.pointC.y) / 3.0f,
                            (triangle.pointA.z + triangle.pointB.z + triangle.pointC.z) / 3.0f
                        };
                        
                        pickData.intersectionPoint = triangleCenter;
                        pickData.intersectionTriangle = triangle;
                        pickData.triangleIndex = j / 3;
                        pickData.collide = true;
                        pickData.node = this;
                    }
                }
            }
        }
    }
    
    if (foundHit && data != nullptr)
    {
        *data = pickData;
    }
    
    return foundHit;
}


static Vector3 GetClosestPointOnTriangle(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c)
{
    Vector3 ab = Vector3Subtract(b, a);
    Vector3 ac = Vector3Subtract(c, a);
    Vector3 ap = Vector3Subtract(point, a);
    
    float d1 = Vector3DotProduct(ab, ap);
    float d2 = Vector3DotProduct(ac, ap);
    
    if (d1 <= 0.0f && d2 <= 0.0f) return a;
    
    Vector3 bp = Vector3Subtract(point, b);
    float d3 = Vector3DotProduct(ab, bp);
    float d4 = Vector3DotProduct(ac, bp);
    
    if (d3 >= 0.0f && d4 <= d3) return b;
    
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
    {
        float v = d1 / (d1 - d3);
        return Vector3Add(a, Vector3Scale(ab, v));
    }
    
    Vector3 cp = Vector3Subtract(point, c);
    float d5 = Vector3DotProduct(ab, cp);
    float d6 = Vector3DotProduct(ac, cp);
    
    if (d6 >= 0.0f && d5 <= d6) return c;
    
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
    {
        float w = d2 / (d2 - d6);
        return Vector3Add(a, Vector3Scale(ac, w));
    }
    
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
    {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Vector3Add(b, Vector3Scale(Vector3Subtract(c, b), w));
    }
    
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    
    return Vector3Add(Vector3Add(a, Vector3Scale(ab, v)), Vector3Scale(ac, w));
}

 
bool Model3D::collide(const Vector3& point, float radius, PickData *data)
{
    if (!m_visible) return false;
    // Create a bounding box around the sphere for quick rejection
    BoundingBox sphereBounds = {
        {point.x - radius, point.y - radius, point.z - radius},
        {point.x + radius, point.y + radius, point.z + radius}
    };
    
    // First check if model's bounding box intersects with the sphere bounds
    if (!CheckCollisionBoxes(world, sphereBounds))
        return false;
    
    PickData pickData;
    bool foundHit = false;
    float closestDistance = radius;
    
    for (int i = 0; i < model->meshCount; i++)
    {
        Mesh mesh = model->meshes[i];
        
        for (int j = 0; j < mesh.triangleCount * 3; j += 3)
        {
            int i0 = mesh.indices[j + 0];
            int i1 = mesh.indices[j + 1];
            int i2 = mesh.indices[j + 2];
            
            Triangle triangle;
            triangle.pointA = {
                mesh.vertices[i0 * 3 + 0],
                mesh.vertices[i0 * 3 + 1],
                mesh.vertices[i0 * 3 + 2]
            };
            triangle.pointB = {
                mesh.vertices[i1 * 3 + 0],
                mesh.vertices[i1 * 3 + 1],
                mesh.vertices[i1 * 3 + 2]
            };
            triangle.pointC = {
                mesh.vertices[i2 * 3 + 0],
                mesh.vertices[i2 * 3 + 1],
                mesh.vertices[i2 * 3 + 2]
            };
            
            // Transform triangle vertices to world space
            triangle.pointA = Vector3Transform(triangle.pointA, this->GetWorldMatrix());
            triangle.pointB = Vector3Transform(triangle.pointB, this->GetWorldMatrix());
            triangle.pointC = Vector3Transform(triangle.pointC, this->GetWorldMatrix());
            triangle.updateBounds();
            
            // Calculate closest point on triangle to the sphere center
            Vector3 closestPoint = GetClosestPointOnTriangle(point, triangle.pointA, triangle.pointB, triangle.pointC);
            float distance = Vector3Distance(point, closestPoint);
            
            if (distance <= radius && distance < closestDistance)
            {
                closestDistance = distance;
                foundHit = true;
                
                pickData.intersectionPoint = closestPoint;
                pickData.intersectionTriangle = triangle;
                pickData.triangleIndex = j / 3;
                pickData.triangleHits++;
                pickData.collide = true;
                pickData.node = this;
            }
        }
    }
    
    if (foundHit && data != nullptr)
    {
        *data = pickData;
    }
    
    return foundHit;
}

std::vector<Triangle> Model3D::GetTriangles(bool transform)
{
    std::vector<Triangle> triangles;

    for (int i = 0; i < model->meshCount; i++)
    {
        Mesh mesh = model->meshes[i];

        for (int j = 0; j < mesh.triangleCount * 3; j += 3)
        {
            int i0 = mesh.indices[j + 0];
            int i1 = mesh.indices[j + 1];
            int i2 = mesh.indices[j + 2];
            
            Triangle triangle;

            triangle.pointA = {
                mesh.vertices[i0 * 3 + 0],
                mesh.vertices[i0 * 3 + 1],
                mesh.vertices[i0 * 3 + 2]
            };

            triangle.pointB = {
                mesh.vertices[i1 * 3 + 0],
                mesh.vertices[i1 * 3 + 1],
                mesh.vertices[i1 * 3 + 2]
            };

            triangle.pointC = {
                mesh.vertices[i2 * 3 + 0],
                mesh.vertices[i2 * 3 + 1],
                mesh.vertices[i2 * 3 + 2]
            };
            triangle.updateBounds();

            if (transform)
            {
                triangle.pointA = Vector3Transform(triangle.pointA, this->GetWorldMatrix());
                triangle.pointB = Vector3Transform(triangle.pointB, this->GetWorldMatrix());
                triangle.pointC = Vector3Transform(triangle.pointC, this->GetWorldMatrix());
            }

            triangles.push_back(triangle);
        }
    }

    return triangles;
}

