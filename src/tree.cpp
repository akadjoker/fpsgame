 
#include "Config.hpp"
#include "collision.hpp"
#include "bsp.hpp"


QuadtreeNode::QuadtreeNode(const BoundingBox& bounds)
    : bounds(bounds), divided(false)
{
    triangles.reserve(MAX_TRIANGLES);
    for (int i = 0; i < 4; i++) children[i] = nullptr;
}

QuadtreeNode::~QuadtreeNode()
{
    for (int i = 0; i < 4; i++)
    {
        delete children[i];
    }
}

void QuadtreeNode::insert(const Triangle* tri, int maxDepth, int depth)
{
    if (!CheckCollisionBoxes(bounds, tri->bounds)) return;

    // Se chegou ao máximo de profundidade ou não está dividido
    if (depth >= maxDepth || !divided)
    {
        triangles.push_back(tri);

        // Subdividir apenas se necessário e possível
        if (triangles.size() > MAX_TRIANGLES && depth < maxDepth)
        {
            Vector3 size = Vector3Subtract(bounds.max, bounds.min);
            if (size.x > MIN_SIZE && size.z > MIN_SIZE)
            {
                subdivide();

          
                auto oldTriangles = std::move(triangles);
                triangles.clear();
                triangles.reserve(MAX_TRIANGLES); // Reserve espaço

                for (const auto& triangle : oldTriangles)
                {
                    bool inserted = false;
                    
            
                    for (int i = 0; i < 4; i++)
                    {
                        if (CheckCollisionBoxes(children[i]->bounds, triangle->bounds))
                        {
                            children[i]->insert(triangle, maxDepth, depth + 1);
                            inserted = true;
              
                        }
                    }
                    
             
                    if (!inserted)
                    {
                        triangles.push_back(triangle);
                    }
                }
            }
        }
    }
    else
    {
         
        bool inserted = false;
        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionBoxes(children[i]->bounds, tri->bounds))
            {
                children[i]->insert(tri, maxDepth, depth + 1);
                inserted = true;
          
            }
        }
        
 
        if (!inserted)
        {
            triangles.push_back(tri);
        }
    }
}

void QuadtreeNode::subdivide()
{
    Vector3 min = bounds.min;
    Vector3 max = bounds.max;
    Vector3 center = { (min.x + max.x) * 0.5f, min.y, (min.z + max.z) * 0.5f };

    BoundingBox quads[4] = {
        { { min.x, min.y, min.z }, { center.x, max.y, center.z } }, // SW
        { { center.x, min.y, min.z }, { max.x, max.y, center.z } }, // SE
        { { min.x, min.y, center.z }, { center.x, max.y, max.z } }, // NW
        { { center.x, min.y, center.z }, { max.x, max.y, max.z } } // NE
    };

    for (int i = 0; i < 4; i++)
    {
        children[i] = new QuadtreeNode(quads[i]);
    }
    divided = true;
}

// QUERIES ULTRA-RÁPIDAS - apenas coletam, sem testes complexos

void QuadtreeNode::collectTriangles(const BoundingBox& area,
                                    std::vector<const Triangle*>& out) const
{
    if (!CheckCollisionBoxes(bounds, area)) return;

    // Adicionar todos os triângulos deste nó
    out.insert(out.end(), triangles.begin(), triangles.end());

    // Recursão nos filhos
    if (divided)
    {
        children[0]->collectTriangles(area, out);
        children[1]->collectTriangles(area, out);
        children[2]->collectTriangles(area, out);
        children[3]->collectTriangles(area, out);
    }
}

void QuadtreeNode::collectTriangles(const Vector3& point, float radius,
                                    std::vector<const Triangle*>& out) const
{
    if (!CheckCollisionBoxSphere(bounds, point, radius)) return;

    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        children[0]->collectTriangles(point, radius, out);
        children[1]->collectTriangles(point, radius, out);
        children[2]->collectTriangles(point, radius, out);
        children[3]->collectTriangles(point, radius, out);
    }
}

void QuadtreeNode::collectTriangles(const Ray& ray, float maxDistance,
                                    std::vector<const Triangle*>& out) const
{
    RayCollision collision = GetRayCollisionBox(ray, bounds);
    if (!collision.hit || collision.distance > maxDistance) return;

    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        children[0]->collectTriangles(ray, maxDistance, out);
        children[1]->collectTriangles(ray, maxDistance, out);
        children[2]->collectTriangles(ray, maxDistance, out);
        children[3]->collectTriangles(ray, maxDistance, out);
    }
}

inline void QuadtreeNode::collectAll(std::vector<const Triangle*>& out) const
{
    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        children[0]->collectAll(out);
        children[1]->collectAll(out);
        children[2]->collectAll(out);
        children[3]->collectAll(out);
    }
}

void QuadtreeNode::clear()
{
    triangles.clear();
    for (int i = 0; i < 4; i++)
    {
        delete children[i];
        children[i] = nullptr;
    }
    divided = false;
}

void QuadtreeNode::debug(Color color) const
{
    DrawBoundingBox(bounds, color);

    if (divided)
    {
        children[0]->debug(GREEN);
        children[1]->debug(YELLOW);
        children[2]->debug(ORANGE);
        children[3]->debug(PURPLE);
    }
}

Quadtree::Quadtree(const BoundingBox& worldBounds)
{
    root = new QuadtreeNode(worldBounds);
}

Quadtree::~Quadtree() { delete root; }

void Quadtree::setWorldBounds(const BoundingBox& bounds) 
{
    if (root) delete root;
    root = new QuadtreeNode(bounds);
}



void Quadtree::addTriangle(const Vector3& a, const Vector3& b, const Vector3& c)
{
    if (!root) return;
    Triangle tri = { a, b, c };
    tri.updateBounds();
    triangleStorage.push_back(tri);
}

void Quadtree::rebuild()
{
    if (!root) return;
    root->clear();
    for (const auto& tri : triangleStorage)
    {
        root->insert(&tri);
    }
}

std::vector<const Triangle*>
Quadtree::getCandidates(const BoundingBox& area) const
{
    if (!root) return {};
    std::vector<const Triangle*> candidates;
    candidates.reserve(64); // Pre-aloca
    root->collectTriangles(area, candidates);
    return candidates;
}

std::vector<const Triangle*> Quadtree::getCandidates(const Vector3& point,
                                                     float radius) const
{
    if (!root) return {};
    std::vector<const Triangle*> candidates;
    candidates.reserve(32);
    root->collectTriangles(point, radius, candidates);
    return candidates;
}

std::vector<const Triangle*> Quadtree::getCandidates(const Ray& ray,
                                                     float maxDistance) const
{
    if (!root) return {};
    std::vector<const Triangle*> candidates;
    candidates.reserve(16);
    root->collectTriangles(ray, maxDistance, candidates);
    return candidates;
}

void Quadtree::debug() const
{
    if (!root) return;
    root->debug();
}


OctreeNode::OctreeNode(const BoundingBox& bounds)
    : bounds(bounds), divided(false)
{
    triangles.reserve(MAX_TRIANGLES);
    for (int i = 0; i < 8; i++) children[i] = nullptr;
}

OctreeNode::~OctreeNode()
{
    for (int i = 0; i < 8; i++)
    {
        delete children[i];
    }
}
void OctreeNode::insert(const Triangle* tri, int maxDepth, int depth)
{
    // Teste rápido de bounds
    if (!CheckCollisionBoxes(bounds, tri->bounds)) return;

    if (depth >= maxDepth || !divided)
    {
        triangles.push_back(tri);

        // Subdividir apenas se necessário e possível
        if (triangles.size() > MAX_TRIANGLES && depth < maxDepth)
        {
            Vector3 size = Vector3Subtract(bounds.max, bounds.min);
            if (size.x > MIN_SIZE && size.y > MIN_SIZE && size.z > MIN_SIZE)
            {
                subdivide();

                // Redistribuir triângulos
                auto oldTriangles = std::move(triangles);
                triangles.clear();

                for (const auto& triangle : oldTriangles)
                {
                    bool inserted = false;
                    for (int i = 0; i < 8; i++)
                    {
                        if (CheckCollisionBoxes(children[i]->bounds, triangle->bounds))
                        {
                            children[i]->insert(triangle, maxDepth, depth + 1);
                            inserted = true;
                        }
                    }

                    if (!inserted)
                    {
                        triangles.push_back(triangle);
                    }
                }
            }
        }
    }
    else
    {
        bool inserted = false;
        for (int i = 0; i < 8; i++)
        {
            if (CheckCollisionBoxes(children[i]->bounds, tri->bounds))
            {
                children[i]->insert(tri, maxDepth, depth + 1);
                inserted = true;
            }
        }

        if (!inserted)
        {
            triangles.push_back(tri);
        }
    }
}


void OctreeNode::subdivide()
{
    Vector3 min = bounds.min;
    Vector3 max = bounds.max;
    Vector3 center = { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f,
                       (min.z + max.z) * 0.5f };

    // 8 octantes: Bottom 4 + Top 4
    BoundingBox octants[8] = {
        // Bottom level (Y = min to center)
        { { min.x, min.y, min.z },
          { center.x, center.y, center.z } }, // 0: Back-Left-Bottom
        { { center.x, min.y, min.z },
          { max.x, center.y, center.z } }, // 1: Back-Right-Bottom
        { { min.x, min.y, center.z },
          { center.x, center.y, max.z } }, // 2: Front-Left-Bottom
        { { center.x, min.y, center.z },
          { max.x, center.y, max.z } }, // 3: Front-Right-Bottom

        // Top level (Y = center to max)
        { { min.x, center.y, min.z },
          { center.x, max.y, center.z } }, // 4: Back-Left-Top
        { { center.x, center.y, min.z },
          { max.x, max.y, center.z } }, // 5: Back-Right-Top
        { { min.x, center.y, center.z },
          { center.x, max.y, max.z } }, // 6: Front-Left-Top
        { { center.x, center.y, center.z },
          { max.x, max.y, max.z } } // 7: Front-Right-Top
    };

    for (int i = 0; i < 8; i++)
    {
        children[i] = new OctreeNode(octants[i]);
    }
    divided = true;
}

// QUERIES ULTRA-RÁPIDAS - apenas coletam, sem testes complexos

void OctreeNode::collectTriangles(const BoundingBox& area,
                                  std::vector<const Triangle*>& out) const
{
    if (!CheckCollisionBoxes(bounds, area)) return;

    // Adicionar todos os triângulos deste nó
    out.insert(out.end(), triangles.begin(), triangles.end());

    // Recursão nos 8 filhos
    if (divided)
    {
        for (int i = 0; i < 8; i++)
        {
            children[i]->collectTriangles(area, out);
        }
    }
}

void OctreeNode::collectTriangles(const Vector3& point, float radius,
                                  std::vector<const Triangle*>& out) const
{
    if (!CheckCollisionBoxSphere(bounds, point, radius)) return;

    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        for (int i = 0; i < 8; i++)
        {
            children[i]->collectTriangles(point, radius, out);
        }
    }
}

void OctreeNode::collectTriangles(const Ray& ray, float maxDistance,
                                  std::vector<const Triangle*>& out) const
{
    RayCollision collision = GetRayCollisionBox(ray, bounds);
    if (!collision.hit || collision.distance > maxDistance) return;

   // DrawBoundingBox(bounds, RED);

    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        for (int i = 0; i < 8; i++)
        {
            children[i]->collectTriangles(ray, maxDistance, out);
        }
    }
}

inline void OctreeNode::collectAll(std::vector<const Triangle*>& out) const
{
    out.insert(out.end(), triangles.begin(), triangles.end());

    if (divided)
    {
        for (int i = 0; i < 8; i++)
        {
            children[i]->collectAll(out);
        }
    }
}

void OctreeNode::clear()
{
    triangles.clear();
    for (int i = 0; i < 8; i++)
    {
        delete children[i];
        children[i] = nullptr;
    }
    divided = false;
}

void OctreeNode::debug(Color color) const
{
    DrawBoundingBox(bounds, color);

    if (divided)
    {
        // Cores diferentes para cada octante
        Color octantColors[8] = {
            RED, // 0: Back-Left-Bottom
            GREEN, // 1: Back-Right-Bottom
            BLUE, // 2: Front-Left-Bottom
            YELLOW, // 3: Front-Right-Bottom
            ORANGE, // 4: Back-Left-Top
            PURPLE, // 5: Back-Right-Top
            PINK, // 6: Front-Left-Top
            GRAY // 7: Front-Right-Top
        };

        for (int i = 0; i < 8; i++)
        {
            children[i]->debug(octantColors[i]);
        }
    }
}

int OctreeNode::getTriangleCount() const
{
    int count = triangles.size();
    if (divided)
    {
        for (int i = 0; i < 8; i++)
        {
            count += children[i]->getTriangleCount();
        }
    }
    return count;
}


Octree::Octree(const BoundingBox& worldBounds) 
{
    root = new OctreeNode(worldBounds);
}

Octree::~Octree() { delete root; }

void Octree::setWorldBounds(const BoundingBox& bounds) 
{
    if (root) delete root;
    root = new OctreeNode(bounds);
}


void Octree::addTriangle(const Vector3& a, const Vector3& b, const Vector3& c)
{
    if (!root) return;
    Triangle tri = { a, b, c };
    tri.updateBounds();
    triangleStorage.push_back(tri);
   
}
    
    void Octree::rebuild() 
    {
        if (!root) return;
        root->clear();
        for (const auto& tri : triangleStorage) 
        {
            root->insert(&tri);
        }
        triangleStorage.clear();
    }
    
   
    std::vector<const Triangle*> Octree::getCandidates(const BoundingBox& area) const {
        if (!root) return {};
        std::vector<const Triangle*> candidates;
        candidates.reserve(64);
        root->collectTriangles(area, candidates);
        return candidates;
    }
    
    std::vector<const Triangle*> Octree::getCandidates(const Vector3& point, float radius) const 
    {
        if (!root) return {};
        std::vector<const Triangle*> candidates;
        candidates.reserve(32);
        root->collectTriangles(point, radius, candidates);
        return candidates;
    }
    
    std::vector<const Triangle*> Octree::getCandidates(const Ray& ray, float maxDistance  ) const 
    {
        if (!root) return {};
        std::vector<const Triangle*> candidates;
        candidates.reserve(16);
        root->collectTriangles(ray, maxDistance, candidates);
        return candidates;
    }
    
    // Query por bounding box do player/objeto
    std::vector<const Triangle*> Octree::getCandidatesForObject(const Vector3& position, const Vector3& size) const 
    {
        BoundingBox objBounds = 
        {
            {position.x - size.x*0.5f, position.y - size.y*0.5f, position.z - size.z*0.5f},
            {position.x + size.x*0.5f, position.y + size.y*0.5f, position.z + size.z*0.5f}
        };
        return getCandidates(objBounds);
    }
    
    void Octree::debug() const 
    {
        if (!root) return;
        root->debug();
    }
 
 
    
    // Helper para estatísticas
    void Octree::stats() const 
    {
        if (!root) return;
        LogInfo("Total Triangles: %d\n", getTriangleCount());
        LogInfo("Octree Triangles: %d\n", root->getTriangleCount());
        LogInfo("Memory efficiency: %.1f%%\n", (float)getTriangleCount() / (float)root->getTriangleCount() * 100.0f);
    }


    