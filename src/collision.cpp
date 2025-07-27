#include "pch.h"
#include "Config.hpp"
#include "collision.hpp"
#include "bsp.hpp"
#include "node.hpp"
#include <cfloat>



Vector3 GetCameraForward(Camera3D camera)
{
    return Vector3Normalize(Vector3Subtract(camera.target, camera.position));
}

Vector3 GetCameraRight(Camera3D camera)
{
    Vector3 forward = GetCameraForward(camera);
    return Vector3Normalize(Vector3CrossProduct(forward, camera.up));
}



Vector4 GetTrianglePlane(const Triangle& triangle)
{
    Vector3 edge1 = Vector3Subtract(triangle.pointB, triangle.pointA);
    Vector3 edge2 = Vector3Subtract(triangle.pointC, triangle.pointA);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

    // Calcula a distância do plano à origem
    float d = -Vector3DotProduct(normal, triangle.pointA);

    return (Vector4){ normal.x, normal.y, normal.z, d };
}

// Função para verificar se um ponto está dentro do triângulo
bool IsPointInsideTriangle(const Triangle& triangle, Vector3 point)
{
    Vector3 v0 = Vector3Subtract(triangle.pointC, triangle.pointA);
    Vector3 v1 = Vector3Subtract(triangle.pointB, triangle.pointA);
    Vector3 v2 = Vector3Subtract(point, triangle.pointA);

    float dot00 = Vector3DotProduct(v0, v0);
    float dot01 = Vector3DotProduct(v0, v1);
    float dot02 = Vector3DotProduct(v0, v2);
    float dot11 = Vector3DotProduct(v1, v1);
    float dot12 = Vector3DotProduct(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

// Função para verificar se o plano está voltado para frente
bool IsFrontFacing(Vector4 plane, Vector3 direction)
{
    Vector3 normal = { plane.x, plane.y, plane.z };
    return Vector3DotProduct(normal, direction) <= 0;
}

// Função para obter a distância do ponto ao plano
float GetDistanceToPlane(Vector4 plane, Vector3 point)
{
    Vector3 normal = { plane.x, plane.y, plane.z };
    return Vector3DotProduct(normal, point) + plane.w;
}

// Função para resolver equação quadrática e obter a menor raiz válida
bool GetLowestRoot(float a, float b, float c, float maxR, float* root)
{
    float determinant = b * b - 4.0f * a * c;

    if (determinant < 0.0f) return false;

    float sqrtD = sqrtf(determinant);
    float r1 = (-b - sqrtD) / (2.0f * a);
    float r2 = (-b + sqrtD) / (2.0f * a);

    if (r1 > r2)
    {
        float temp = r1;
        r1 = r2;
        r2 = temp;
    }

    if (r1 > 0 && r1 < maxR)
    {
        *root = r1;
        return true;
    }

    if (r2 > 0 && r2 < maxR)
    {
        *root = r2;
        return true;
    }

    return false;
}


// Função principal para testar interseção esfera-triângulo
bool TestTriangleIntersection(CollisionData* colData, const Triangle& triangle)
{
    Vector4 trianglePlane = GetTrianglePlane(triangle);

    // Verifica apenas polígonos voltados para frente
    if (!IsFrontFacing(trianglePlane, colData->normalizedVelocity))
    {
        return false;
    }

    // Obtém intervalo de interseção do plano
    float t1, t0;
    bool embeddedInPlane = false;

    // Calcula distância com sinal da posição da esfera ao plano do triângulo
    float signedDistToTrianglePlane =
        GetDistanceToPlane(trianglePlane, colData->basePoint);

    Vector3 planeNormal = { trianglePlane.x, trianglePlane.y, trianglePlane.z };
    float normalDotVelocity = Vector3DotProduct(planeNormal, colData->velocity);

    if (fabsf(normalDotVelocity) < 0.0001f)
    {
        // Esfera viajando paralela ao plano
        if (fabsf(signedDistToTrianglePlane) >= 1.0f)
        {
            return false; // Sem colisão possível
        }
        else
        {
            // Esfera está incorporada no plano
            embeddedInPlane = true;
            t0 = 0.0f;
            t1 = 1.0f;
        }
    }
    else
    {
        float invNormalDotVelocity = 1.0f / normalDotVelocity;

        // N.D não é 0. Calcula intervalo de interseção
        t0 = (-1.0f - signedDistToTrianglePlane) * invNormalDotVelocity;
        t1 = (1.0f - signedDistToTrianglePlane) * invNormalDotVelocity;

        // Troca para que t0 < t1
        if (t0 > t1)
        {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }

        // Verifica se pelo menos um valor está dentro do intervalo
        if (t0 > 1.0f || t1 < 0.0f)
        {
            return false; // Ambos os valores t estão fora de [0,1], sem colisão
                          // possível
        }

        // Limita a 0 e 1
        t0 = Clamp(t0, 0.0f, 1.0f);
        t1 = Clamp(t1, 0.0f, 1.0f);
    }

    // Neste ponto temos t0 e t1, se há alguma interseção,
    // está entre este intervalo
    Vector3 collisionPoint = { 0 };
    bool foundCollision = false;
    float t = 1.0f;

    // Primeiro verifica o caso fácil: Colisão dentro do triângulo
    if (!embeddedInPlane)
    {
        Vector3 planeIntersectionPoint =
            Vector3Add(Vector3Subtract(colData->basePoint, planeNormal),
                       Vector3Scale(colData->velocity, t0));

        if (IsPointInsideTriangle(triangle, planeIntersectionPoint))
        {
            foundCollision = true;
            t = t0;
            collisionPoint = planeIntersectionPoint;
        }
    }

    // Se não encontramos colisão, precisamos varrer a esfera
    // contra pontos e arestas do triângulo
    if (!foundCollision)
    {
        Vector3 velocity = colData->velocity;
        Vector3 base = colData->basePoint;

        float velocitySquaredLength = Vector3DotProduct(velocity, velocity);
        float a, b, c;
        float newT;

        // Para cada aresta ou vértice, uma equação quadrática deve ser
        // resolvida: a*t^2 + b*t + c = 0

        // Verifica contra pontos
        a = velocitySquaredLength;

        // Ponto A
        Vector3 baseToPointA = Vector3Subtract(base, triangle.pointA);
        b = 2.0f * Vector3DotProduct(velocity, baseToPointA);
        Vector3 distToA = Vector3Subtract(triangle.pointA, base);
        c = Vector3DotProduct(distToA, distToA) - 1.0f;
        if (GetLowestRoot(a, b, c, t, &newT))
        {
            t = newT;
            foundCollision = true;
            collisionPoint = triangle.pointA;
        }

        // Ponto B
        if (!foundCollision)
        {
            Vector3 baseToPointB = Vector3Subtract(base, triangle.pointB);
            b = 2.0f * Vector3DotProduct(velocity, baseToPointB);
            Vector3 distToB = Vector3Subtract(triangle.pointB, base);
            c = Vector3DotProduct(distToB, distToB) - 1.0f;
            if (GetLowestRoot(a, b, c, t, &newT))
            {
                t = newT;
                foundCollision = true;
                collisionPoint = triangle.pointB;
            }
        }

        // Ponto C
        if (!foundCollision)
        {
            Vector3 baseToPointC = Vector3Subtract(base, triangle.pointC);
            b = 2.0f * Vector3DotProduct(velocity, baseToPointC);
            Vector3 distToC = Vector3Subtract(triangle.pointC, base);
            c = Vector3DotProduct(distToC, distToC) - 1.0f;
            if (GetLowestRoot(a, b, c, t, &newT))
            {
                t = newT;
                foundCollision = true;
                collisionPoint = triangle.pointC;
            }
        }

        // Verifica contra arestas:

        // Aresta A-B
        Vector3 edge = Vector3Subtract(triangle.pointB, triangle.pointA);
        Vector3 baseToVertex = Vector3Subtract(triangle.pointA, base);
        float edgeSquaredLength = Vector3DotProduct(edge, edge);
        float edgeDotVelocity = Vector3DotProduct(edge, velocity);
        float edgeDotBaseToVertex = Vector3DotProduct(edge, baseToVertex);

        a = edgeSquaredLength * -velocitySquaredLength
            + edgeDotVelocity * edgeDotVelocity;
        b = edgeSquaredLength
                * (2.0f * Vector3DotProduct(velocity, baseToVertex))
            - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
        c = edgeSquaredLength
                * (1.0f - Vector3DotProduct(baseToVertex, baseToVertex))
            + edgeDotBaseToVertex * edgeDotBaseToVertex;

        if (GetLowestRoot(a, b, c, t, &newT))
        {
            float f = (edgeDotVelocity * newT - edgeDotBaseToVertex)
                / edgeSquaredLength;
            if (f >= 0.0f && f <= 1.0f)
            {
                t = newT;
                foundCollision = true;
                collisionPoint =
                    Vector3Add(triangle.pointA, Vector3Scale(edge, f));
            }
        }

        // Aresta B-C
        edge = Vector3Subtract(triangle.pointC, triangle.pointB);
        baseToVertex = Vector3Subtract(triangle.pointB, base);
        edgeSquaredLength = Vector3DotProduct(edge, edge);
        edgeDotVelocity = Vector3DotProduct(edge, velocity);
        edgeDotBaseToVertex = Vector3DotProduct(edge, baseToVertex);

        a = edgeSquaredLength * -velocitySquaredLength
            + edgeDotVelocity * edgeDotVelocity;
        b = edgeSquaredLength
                * (2.0f * Vector3DotProduct(velocity, baseToVertex))
            - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
        c = edgeSquaredLength
                * (1.0f - Vector3DotProduct(baseToVertex, baseToVertex))
            + edgeDotBaseToVertex * edgeDotBaseToVertex;

        if (GetLowestRoot(a, b, c, t, &newT))
        {
            float f = (edgeDotVelocity * newT - edgeDotBaseToVertex)
                / edgeSquaredLength;
            if (f >= 0.0f && f <= 1.0f)
            {
                t = newT;
                foundCollision = true;
                collisionPoint =
                    Vector3Add(triangle.pointB, Vector3Scale(edge, f));
            }
        }

        // Aresta C-A
        edge = Vector3Subtract(triangle.pointA, triangle.pointC);
        baseToVertex = Vector3Subtract(triangle.pointC, base);
        edgeSquaredLength = Vector3DotProduct(edge, edge);
        edgeDotVelocity = Vector3DotProduct(edge, velocity);
        edgeDotBaseToVertex = Vector3DotProduct(edge, baseToVertex);

        a = edgeSquaredLength * -velocitySquaredLength
            + edgeDotVelocity * edgeDotVelocity;
        b = edgeSquaredLength
                * (2.0f * Vector3DotProduct(velocity, baseToVertex))
            - 2.0f * edgeDotVelocity * edgeDotBaseToVertex;
        c = edgeSquaredLength
                * (1.0f - Vector3DotProduct(baseToVertex, baseToVertex))
            + edgeDotBaseToVertex * edgeDotBaseToVertex;

        if (GetLowestRoot(a, b, c, t, &newT))
        {
            float f = (edgeDotVelocity * newT - edgeDotBaseToVertex)
                / edgeSquaredLength;
            if (f >= 0.0f && f <= 1.0f)
            {
                t = newT;
                foundCollision = true;
                collisionPoint =
                    Vector3Add(triangle.pointC, Vector3Scale(edge, f));
            }
        }
    }

    // Define resultado:
    if (foundCollision)
    {
        // Distância para colisão é t
        float distToCollision = t * Vector3Length(colData->velocity);

        // Este triângulo qualifica para o hit mais próximo?
        if (!colData->foundCollision
            || distToCollision < colData->nearestDistance)
        {
            colData->nearestDistance = distToCollision;
            colData->intersectionPoint = collisionPoint;
            colData->foundCollision = true;
            colData->triangleHits++;
            return true;
        }
    }

    return false;
}


void Collider::setCollisionSelector(Selector* selector) 
{
    if (selector) 
    {
        collisionSelector = selector;
    }
}

void Collider::setScene(Scene* scene) 
{
    if (scene) 
    {
        this->scene = scene;
    }
}


Vector3 Collider::collideWithWorld(s32 recursionDepth, CollisionData& colData,Vector3 pos, Vector3 vel)
{
    float veryCloseDistance = colData.slidingSpeed;

    if (recursionDepth > 3) return pos;

    colData.velocity = vel;
    colData.normalizedVelocity = vel;
    colData.normalizedVelocity = Vector3Normalize(colData.normalizedVelocity);
    colData.basePoint = pos;
    colData.foundCollision = false;
    colData.nearestDistance = FLT_MAX;

    Matrix scale = MatrixScale(1.0f / colData.eRadius.x, 1.0f / colData.eRadius.y,1.0f / colData.eRadius.z);

    if(!collisionSelector && !scene) return Vector3Add(pos, vel);


    Vector3 currentPos = colData.R3Position;
    Vector3 targetPos = Vector3Add(colData.R3Position, colData.R3Velocity);
    
    BoundingBox queryBox;

    queryBox.min = Vector3Min(currentPos, targetPos);
    queryBox.max = Vector3Max(currentPos, targetPos);
    
 
    
 
    Vector3 maxRadius = 
    {
        fmaxf(fmaxf(colData.eRadius.x, colData.eRadius.y), colData.eRadius.z),// * 2.0f,
        fmaxf(fmaxf(colData.eRadius.x, colData.eRadius.y), colData.eRadius.z),// * 2.0f,
        fmaxf(fmaxf(colData.eRadius.x, colData.eRadius.y), colData.eRadius.z)// * 2.0f
    };
    
    queryBox.min = Vector3Subtract(queryBox.min, maxRadius);
    queryBox.max = Vector3Add(queryBox.max, maxRadius);

 
  //  std::vector<const Triangle*> triangles = collisionSelector->getCandidates(queryBox);

    std::vector<const Triangle*> triangles;
 
    
    
    if (scene) 
    {
        auto sceneTriangles = scene->collectTriangles(queryBox);
        triangles.insert(triangles.end(), sceneTriangles.begin(), sceneTriangles.end());
        
       
    }
    
    // Da octree (se disponível)
    if (collisionSelector) 
    {
        auto octreeTriangles = collisionSelector->getCandidates(queryBox);
        triangles.insert(triangles.end(), octreeTriangles.begin(), octreeTriangles.end());
    }


 
    u32 triangleCnt = triangles.size();
    
	for (u32 i=0; i<triangleCnt; ++i)
  //  for (const Triangle* tri : triangles) 
    {
        Triangle t = *triangles[i];
        t.pointA = Vector3Transform(t.pointA, scale);
        t.pointB = Vector3Transform(t.pointB, scale);
        t.pointC = Vector3Transform(t.pointC, scale);
        
        
        if (TestTriangleIntersection(&colData, t))
        {
            colData.triangleIndex = i;
        }
        
       
    }


     // DrawBoundingBox(queryBox, LIME);
    
//     // Desenhar posição atual
//   //  DrawSphere(colData.R3Position, 0.5f, BLUE);
    
//     // Desenhar triângulos candidatos
//     for (const Triangle* tri : triangles) 
//     {
//       //  DrawTriangle3D(tri->pointA, tri->pointB, tri->pointC, RED);
//         DrawBoundingBox(tri->bounds, ORANGE);
//     }
    
//     // Se houve colisão, desenhar ponto de interseção
//     // if (colData.foundCollision)
//     // {
//     //     DrawSphere(colData.intersectionPoint, 0.3f, YELLOW);
//     // }


    if (!colData.foundCollision) return Vector3Add(pos, vel); // pos + vel;

    // original destination point
    const Vector3 destinationPoint = Vector3Add(pos, vel); // pos + vel;
    Vector3 newBasePoint = pos;


    if (colData.nearestDistance >= veryCloseDistance)
    {

         float moveDistance = colData.nearestDistance - veryCloseDistance;
        if (moveDistance > 0.0f)
        {
            Vector3 moveVector = Vector3Scale(Vector3Normalize(vel), moveDistance);
            newBasePoint = Vector3Add(pos, moveVector);
            Vector3 moveDirection = Vector3Normalize(vel);
            colData.intersectionPoint = Vector3Subtract(colData.intersectionPoint, Vector3Scale(moveDirection, veryCloseDistance));
        }
 
    }


    // // calculate sliding plane

    // 1. Definir o plano de deslizamento
    Vector3 slidePlaneOrigin = colData.intersectionPoint;
    Vector3 slidePlaneNormal = Vector3Normalize(Vector3Subtract(colData.intersectionPoint, colData.basePoint));

    // 2. Representar plano como vetor 4D (Ax + By + Cz + D = 0)
    Vector4 slidingPlane = 
    {
        slidePlaneNormal.x, slidePlaneNormal.y, slidePlaneNormal.z,-Vector3DotProduct(slidePlaneNormal, slidePlaneOrigin)
    };

    if (Vector3Length(slidePlaneNormal) < 0.1f)
    {
        
            return newBasePoint;
        
    }



    float distToPlane =Vector3DotProduct(slidePlaneNormal, destinationPoint) + slidingPlane.w;


    Vector3 newDestinationPoint = Vector3Subtract(destinationPoint, Vector3Scale(slidePlaneNormal, distToPlane));


    Vector3 newVelocityVector =Vector3Subtract(newDestinationPoint, colData.intersectionPoint);


    float newVelLength = Vector3Length(newVelocityVector);
    if (newVelLength < veryCloseDistance)
    {
        return newBasePoint;
    }
    
    newVelocityVector = Vector3Scale(newVelocityVector, 0.95f);


    return collideWithWorld(recursionDepth + 1, colData, newBasePoint,newVelocityVector);
}


Vector3 Collider::collideEllipsoidWithWorld(
    const Vector3& position, const Vector3& radius, const Vector3& velocity,
    float slidingSpeed, const Vector3& gravity, Triangle& triout,
    Vector3& hitPosition, bool& outFalling, bool& outCollide)
{

    if (radius.x == 0.0f || radius.y == 0.0f || radius.z == 0.0f)
        return position;

    // This code is based on the paper "Improved Collision detection
    // andResponse" by Kasper Fauerby, but some parts are modified.

    CollisionData colData;
    colData.R3Position = position;
    colData.R3Velocity = velocity;
    colData.eRadius = radius;
    colData.nearestDistance = FLT_MAX;
    colData.slidingSpeed = slidingSpeed;
    colData.triangleHits = 0;
    colData.triangleIndex = -1;

    Vector3 eSpacePosition = Vector3Divide(colData.R3Position, colData.eRadius);
    Vector3 eSpaceVelocity = Vector3Divide(colData.R3Velocity, colData.eRadius);


    // iterate until we have our final position

    Vector3 finalPos =
        collideWithWorld(0, colData, eSpacePosition, eSpaceVelocity);


    outFalling = false;

    // add gravity

    if (gravity.x != 0.0f || gravity.y != 0.0f || gravity.z != 0.0f)
    {
        colData.R3Position = Vector3Multiply(finalPos, colData.eRadius);
        colData.R3Velocity = gravity;
        colData.triangleHits = 0;

        eSpaceVelocity = Vector3Divide(gravity, colData.eRadius);

        finalPos = collideWithWorld(0, colData, finalPos, eSpaceVelocity);

        outFalling = (colData.triangleHits == 0);
    }

    if (colData.triangleHits)
    {
        triout = colData.intersectionTriangle;
        triout.pointA.x *= colData.eRadius.x;
        triout.pointA.y *= colData.eRadius.y;
        triout.pointA.z *= colData.eRadius.z;
        triout.pointB.x *= colData.eRadius.x;
        triout.pointB.y *= colData.eRadius.y;
        triout.pointB.z *= colData.eRadius.z;
        triout.pointC.x *= colData.eRadius.x;
        triout.pointC.y *= colData.eRadius.y;
        triout.pointC.z *= colData.eRadius.z;
    }

    outCollide = (colData.triangleHits > 0);

    finalPos.x *= colData.eRadius.x;
    finalPos.y *= colData.eRadius.y;
    finalPos.z *= colData.eRadius.z;
    hitPosition = Vector3Multiply(colData.intersectionPoint, colData.eRadius);
    return finalPos;
}

  
bool intersects(const BoundingBox& a, const BoundingBox& b)
{
    return CheckCollisionBoxes(a, b);
}

bool intersects(const BoundingBox& box, const Ray& ray)
{
    RayCollision col = GetRayCollisionBox(ray, box);
    return col.hit;
}

bool intersects(const BoundingBox& box, const Vector3& center,
                              float radius)
{
    return CheckCollisionBoxSphere(box, center, radius);
}


    Vector3 CalculateTriangleNormal(Vector3 v1, Vector3 v2, Vector3 v3)
    {
        Vector3 edge1 = Vector3Subtract(v2, v1);
        Vector3 edge2 = Vector3Subtract(v3, v1);
        Vector3 normal = Vector3CrossProduct(edge1, edge2);
        return Vector3Normalize(normal);
    }

void GetTriangleInfo(Vector3 v1, Vector3 v2, Vector3 v3, Vector3& center, Vector3& normal)
{
    
    center = Vector3Scale(Vector3Add(Vector3Add(v1, v2), v3), 1.0f / 3.0f);
    
   
    Vector3 edge1 = Vector3Subtract(v2, v1);
    Vector3 edge2 = Vector3Subtract(v3, v1);
    normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
}


void CalculateSurfaceOrientation(Vector3 hitPoint, const Triangle* hitTriangle, 
                                Vector3& tangent, Vector3& bitangent, Vector3 normal)
{
    Vector3 edge1 = Vector3Subtract(hitTriangle->pointB, hitTriangle->pointA);
    Vector3 edge2 = Vector3Subtract(hitTriangle->pointC, hitTriangle->pointA);
    
    // Usa a aresta mais longa como tangent principal
    float len1 = Vector3Length(edge1);
    float len2 = Vector3Length(edge2);
    
    if (len1 > len2) {
        tangent = Vector3Normalize(edge1);
        // Projeta edge2 no plano para criar bitangent ortogonal
        Vector3 edge2Projected = Vector3Subtract(edge2, Vector3Scale(tangent, Vector3DotProduct(edge2, tangent)));
        bitangent = Vector3Normalize(edge2Projected);
    } else {
        tangent = Vector3Normalize(edge2);
        // Projeta edge1 no plano para criar bitangent ortogonal
        Vector3 edge1Projected = Vector3Subtract(edge1, Vector3Scale(tangent, Vector3DotProduct(edge1, tangent)));
        bitangent = Vector3Normalize(edge1Projected);
    }
    
    // Garante que temos um sistema destro
    if (Vector3DotProduct(Vector3CrossProduct(tangent, bitangent), normal) < 0) {
        bitangent = Vector3Negate(bitangent);
    }
}

   void CalculateTangentBitangent(Vector3 normal, Vector3& tangent, Vector3& bitangent)
    {
        // Encontra um vetor que não seja paralelo à normal
        Vector3 up = { 0, 1, 0 };
        if (fabs(Vector3DotProduct(normal, up)) > 0.9f)
        {
            up = { 1, 0,0 }; // Use outro eixo se a normal for muito próxima do Y
        }

        tangent = Vector3CrossProduct(normal, up);
        tangent = Vector3Normalize(tangent);
        bitangent = Vector3CrossProduct(normal, tangent);
        bitangent = Vector3Normalize(bitangent);
    }

    // Verifica se um ponto está dentro de um triângulo usando coordenadas
    // baricêntricas
    bool IsPointInTriangle(Vector3 point, Vector3 v1, Vector3 v2, Vector3 v3,Vector3& baryCoords)
    {
        Vector3 v0 = Vector3Subtract(v3, v1);
        Vector3 v1_new = Vector3Subtract(v2, v1);
        Vector3 v2_new = Vector3Subtract(point, v1);

        float dot00 = Vector3DotProduct(v0, v0);
        float dot01 = Vector3DotProduct(v0, v1_new);
        float dot02 = Vector3DotProduct(v0, v2_new);
        float dot11 = Vector3DotProduct(v1_new, v1_new);
        float dot12 = Vector3DotProduct(v1_new, v2_new);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        baryCoords = { 1.0f - u - v, v, u };

        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }