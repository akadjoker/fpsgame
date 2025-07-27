#pragma once
// #include "raylib.h"
// #include "raymath.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <vector>
// #include <math.h>

class BSPSurface;

#define MAX_RECURSION 5


struct Triangle
{
    Vector3 pointA;
    Vector3 pointB;
    Vector3 pointC;
    BoundingBox bounds;
    Vector3 normal; 

    void updateBounds()
    {
        bounds.min = Vector3Min(Vector3Min(pointA, pointB), pointC);
        bounds.max = Vector3Max(Vector3Max(pointA, pointB), pointC);
        


        bounds.min = Vector3Subtract(bounds.min, {EPSILON, EPSILON, EPSILON});
        bounds.max = Vector3Add(bounds.max, {EPSILON, EPSILON, EPSILON});
        

        Vector3 edge1 = Vector3Subtract(pointB, pointA);
        Vector3 edge2 = Vector3Subtract(pointC, pointA);
        normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
    }
};


struct CollisionData
{
    Vector3 eRadius;

    Vector3 R3Velocity;
    Vector3 R3Position;

    Vector3 velocity;
    Vector3 normalizedVelocity;
    Vector3 basePoint;

    bool foundCollision;
    float nearestDistance;
    Vector3 intersectionPoint;

    Triangle intersectionTriangle;
    s32 triangleIndex;
    s32 triangleHits;

    float slidingSpeed;
};
class QuadtreeNode {
private:
    BoundingBox bounds;
    std::vector<const Triangle*> triangles;
    QuadtreeNode* children[4];
    bool divided;

    static constexpr int MAX_TRIANGLES = 16;
    static constexpr float MIN_SIZE = 0.5f;

public:
    QuadtreeNode(const BoundingBox& bounds);
    ~QuadtreeNode();

    void insert(const Triangle* tri, int maxDepth = 8, int depth = 0);
    void subdivide();


    void collectTriangles(const BoundingBox& area,
                          std::vector<const Triangle*>& out) const;
    void collectTriangles(const Vector3& point, float radius,
                          std::vector<const Triangle*>& out) const;
    void collectTriangles(const Ray& ray, float maxDistance,
                          std::vector<const Triangle*>& out) const;


    inline void collectAll(std::vector<const Triangle*>& out) const;

    void clear();
    void debug(Color color = BLUE) const;


    int getTriangleCount() const
    {
        return triangles.size()
            + (divided ? children[0]->getTriangleCount()
                       + children[1]->getTriangleCount()
                       + children[2]->getTriangleCount()
                       + children[3]->getTriangleCount()
                       : 0);
    }
};


class OctreeNode {
private:
    BoundingBox bounds;
    std::vector<const Triangle*> triangles;
    OctreeNode* children[8];
    bool divided;

    static constexpr int MAX_TRIANGLES = 16; 
    static constexpr float MIN_SIZE = 0.5f;

public:
    OctreeNode(const BoundingBox& bounds);
    ~OctreeNode();

    void insert(const Triangle* tri, int maxDepth = 8, int depth = 0);
    void subdivide();


    void collectTriangles(const BoundingBox& area,
                          std::vector<const Triangle*>& out) const;
    void collectTriangles(const Vector3& point, float radius,
                          std::vector<const Triangle*>& out) const;
    void collectTriangles(const Ray& ray, float maxDistance,
                          std::vector<const Triangle*>& out) const;


    inline void collectAll(std::vector<const Triangle*>& out) const;

    void clear();
    void debug(Color color = BLUE) const;

    int getTriangleCount() const;
};


class Selector 
{

protected:
   std::vector<Triangle> triangleStorage;    
public:
        virtual ~Selector() = default;



    virtual void addTriangle(const Vector3& a, const Vector3& b, const Vector3& c)=0;
    virtual void rebuild() =0;


   virtual  std::vector<const Triangle*> getCandidates(const BoundingBox& area) const = 0;
   virtual std::vector<const Triangle*> getCandidates(const Vector3& point,
                                                      float radius) const = 0;
   virtual std::vector<const Triangle*> getCandidates(const Ray& ray, float maxDistance = 1000.0f) const = 0;
   
   virtual void debug() const =0;
   
    int getTriangleCount() const { return triangleStorage.size(); }
};


class Quadtree : public Selector
 {
private:
    QuadtreeNode* root;
    std::vector<Triangle> triangleStorage;

public:
    Quadtree()=default;
    Quadtree(const BoundingBox& worldBounds);

    ~Quadtree();

    void setWorldBounds(const BoundingBox& bounds);


    void addTriangle(const Vector3& a, const Vector3& b, const Vector3& c);
     void rebuild();


    std::vector<const Triangle*> getCandidates(const BoundingBox& area) const ;
    std::vector<const Triangle*> getCandidates(const Vector3& point,float radius) const ;
    std::vector<const Triangle*> getCandidates(const Ray& ray, float maxDistance = 1000.0f) const ;
   
    void debug() const ;




   
    
};


class Octree :  public Selector
{
private:
    OctreeNode* root;


public:
    Octree()=default;
    Octree(const BoundingBox& worldBounds);
    ~Octree();

    void setWorldBounds(const BoundingBox& bounds) ;

    void addTriangle(const Vector3& a, const Vector3& b, const Vector3& c);
    void rebuild();


    std::vector<const Triangle*> getCandidates(const BoundingBox& area) const;
    std::vector<const Triangle*> getCandidates(const Vector3& point,float radius) const;
    std::vector<const Triangle*> getCandidates(const Ray& ray, float maxDistance = 1000.0f) const;
    std::vector<const Triangle*> getCandidatesForObject(const Vector3& position, const Vector3& size) const;
 

    void debug() const;

 
 

    void stats() const;
};


struct Collider
{
private:
   
    Selector* collisionSelector{nullptr}; 
public:
    void setCollisionSelector(Selector* selector);
 
    Vector3 collideWithWorld(s32 recursionDepth, CollisionData& colData,
                             Vector3 pos, Vector3 vel);
    Vector3 collideEllipsoidWithWorld(
        const Vector3& position, const Vector3& radius, const Vector3& velocity,
        float slidingSpeed, const Vector3& gravity, Triangle& triout,
        Vector3& hitPosition, bool& outFalling, bool& outCollide);
};


Vector3 GetCameraForward(Camera3D camera);

Vector3 GetCameraRight(Camera3D camera);

bool TestTriangleIntersection(CollisionData* colData, const Triangle& triangle);
Vector3 CalculateTriangleNormal(Vector3 v1, Vector3 v2, Vector3 v3);
void GetTriangleInfo(Vector3 v1, Vector3 v2, Vector3 v3, Vector3& center,
                     Vector3& normal);


void CalculateSurfaceOrientation(Vector3 hitPoint, const Triangle* hitTriangle,
                                 Vector3& tangent, Vector3& bitangent,
                                 Vector3 normal);