 
#include "scene.hpp"
#include "collision.hpp"
#include "node.hpp"


static u32 IDS = 0;


void Scene::AddToRemove(Model3D* node) 
{
    if (node)
    {
        toRemove.push_back(node);
    }

}

Model3D* Scene::AddNode(Model* model)
{
     Model3D* newNode = new Model3D(model);
     newNode->ID = IDS++;
     this->nodes.push_back(newNode);
     return newNode;
}

Model3D* Scene::AddNode(Model* model, Vector3 position) 
{
    Model3D *newNode = new Model3D(model);
    newNode->ID = IDS++;
    newNode->localPosition = position;
    newNode->GetWorldMatrix();
    this->nodes.push_back(newNode);
    return newNode;
}

Model3D* Scene::AddNode(Model* model, Vector3 position, Vector3 scale)
{
    Model3D *newNode = new Model3D(model);
    newNode->ID = IDS++;
    newNode->localPosition = position;
    newNode->localScale = scale;
    newNode->GetWorldMatrix();
    this->nodes.push_back(newNode);
    return newNode;
}


void Scene::RemoveNode(Model3D* node)
{
    auto it = std::find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        nodes.erase(it);
    }
}

void Scene::Clear() 
{
    for (auto& node : nodes)
    {
        delete node;
    }
    nodes.clear();
}





bool Scene::collide(const Ray& ray, float maxDistance, PickData* data)  
{
    if (nodes.empty()) return false;

    for (auto& node : nodes)
    {
        if (node->collide(ray, maxDistance, data)) return true;
    }
   
    return false;
   
}



bool Scene::collide(const Vector3& point, float radius, PickData *data) 
{
    if (nodes.empty()) return false;

    for (auto& node : nodes)
    {
        if (node->collide(point, radius, data)) return true;
    }
   
    return false;
}
 
bool Scene::collide(const BoundingBox& area, PickData* data) 
{
    if (nodes.empty()) return false;

    for (auto& node : nodes)
    {
        if (node->collide(area, data)) return true;
    }
   
    return false;
}

std::vector<const Triangle*> Scene::collectTriangles(const BoundingBox& area) const
{
    std::vector<const Triangle*> triangles;
    for (auto& node : nodes)
    {
        node->collectTriangles(area, triangles);
    }
    return triangles;
}




void Scene::Update(float dt) 
{
    for (auto& node : nodes)
    {
        node->Update(dt);
    }

    for (auto& node : toRemove)
    {
        RemoveNode(node);
    }
    toRemove.clear();

}

void Scene::Render() 
{

    for (auto& node : nodes)
    {
        node->Render();
    }
}