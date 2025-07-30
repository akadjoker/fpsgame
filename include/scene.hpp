#pragma once
#include "Config.hpp"
#include "collision.hpp"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <algorithm>

class Node3D;
class Model3D;
struct PickData;

class Scene
{
    std::vector<Model3D*> nodes;
    std::vector<Model3D*> toRemove;


public:
    void AddToRemove(Model3D* node);

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