#ifndef BSP_LOADER_H
#define BSP_LOADER_H

// #include <raylib.h>
// #include <raymath.h>
// #include <stdint.h>
#include "binaryfile.hpp"

class ViewFrustum;

struct BSPLump
{
    s32 offset;
    s32 length;
};

struct BSPHeader
{
    s32 strID; // This should always be 'IBSP'
    s32 version; // This should be 0x2e for Quake 3 files
};


struct BSPTexture
{
    c8 strName[64]; // The name of the texture w/o the extension
    u32 flags; // The surface flags (unknown)
    u32 contents; // The content flags (unknown)
};

struct BSPLightmap
{
    u8 imageBits[128][128][3]; // The RGB data in a 128x128 image
};

struct BSPVertex
{
    Vector3 vPosition; // (x, y, z) position.
    Vector2 vTextureCoord; // (u, v) texture coordinate
    Vector2 vLightmapCoord; // (u, v) lightmap coordinate
    Vector3 vNormal; // (x, y, z) normal vector
    u8 color[4]; // RGBA color for the vertex
};

// Face BSP
struct BSPFace
{
    s32 textureID; // The index into the texture array
    s32 effect; // The index for the effects (or -1 = n/a)
    s32 type; // 1=polygon, 2=patch, 3=mesh, 4=billboard
    s32 startVertIndex; //   The index into this face's first vertex
    s32 numOfVerts; // The number of vertices for this face
    s32 startIndex; // The index into the first meshvertex
    s32 numOfIndices; // The number of mesh vertices
    s32 lightmapID; // The texture index for the lightmap
    s32 lMapCorner[2]; // The face's lightmap corner in the image
    s32 lMapSize[2]; // The size of the lightmap section
    float lMapPos[3]; // The 3D origin of lightmap.
    float lMapBitsets[2][3]; // The 3D space for s and t unit vectors.
    float vNormal[3]; // The face normal.
    s32 size[2]; // The bezier patch dimensions.
};


struct BSPNode
{
    s32 plane; // The index into the planes array
    s32 front; // The child index for the front node
    s32 back; // The child index for the back node
    s32 mins[3]; // The bounding box min position.
    s32 maxs[3]; // The bounding box max position.
};

struct BSPLeaf
{
    s32 cluster; // The visibility cluster
    s32 area; // The area portal
    s32 mins[3]; // The bounding box min position
    s32 maxs[3]; // The bounding box max position
    s32 leafface; // The first index into the face array
    s32 numOfLeafFaces; // The number of faces for this leaf
    s32 leafBrush; // The first index for into the brushes
    s32 numOfLeafBrushes; // The number of brushes for this leaf
};

struct BSPPlane
{
    float vNormal[3]; // Plane normal.
    float d; // The plane distance from origin
};

struct BSPVisData
{
    s32 numOfClusters; // The number of clusters
    s32 bytesPerCluster; // Bytes (8 bits) in the cluster's bitset
    c8* pBitsets; // Array of bytes holding the cluster vis.
};

struct BSPBrush
{
    s32 brushSide; // The starting brush side for the brush
    s32 numOfBrushSides; // Number of brush sides for the brush
    s32 textureID; // The texture index for the brush
};

struct BSPBrushSide
{
    s32 plane; // The plane index
    s32 textureID; // The texture index
};

struct BSPModel
{
    float min[3]; // The min position for the bounding box
    float max[3]; // The max position for the bounding box.
    s32 faceIndex; // The first face index in the model
    s32 numOfFaces; // The number of faces in the model
    s32 brushIndex; // The first brush index in the model
    s32 numOfBrushes; // The number brushes for the model
};

struct BSPFog
{
    c8 shader[64]; // The name of the shader file
    s32 brushIndex; // The brush index for this shader
    s32 visibleSide; // the brush side that ray tests need to clip against (-1
                     // == none
};

struct BspEntity
{
    std::vector<std::string> name;
    std::vector<std::string> value;
};


struct BSPSurface
{
    u32 vertexCount { 0 };
    u32 triangleCount{ 0 }; 

    unsigned int vaoId{ 0 };    
    unsigned int vboId[6] { 0, 0, 0, 0, 0 };    

    s32 textureID{ 0 };
    s32 lightmapID{ 0 };
    std::vector<Vector2> uv0;
    std::vector<Vector2> uv1;
    std::vector<Vector3> normals;
    std::vector<Vector3> vertices;
    std::vector<Color> colors;
    std::vector<u16> indices;
    BoundingBox bounds;
    void createCube();
    void init();
    void clear();
    void update();
    void render();
    void updateBounds();
};


class BSP {

private:
    enum
    {
        kEntities = 0,
        kTextures,
        kPlanes,
        kNodes,
        kLeafs,
        kLeafFaces,
        kLeafBrushes,
        kModels,
        kBrushes,
        kBrushSides,
        kVertices,
        kIndices,
        kShaders,
        kFaces,
        kLightmaps,
        kLightVolumes,
        kVisData,
        kMaxLumps
    };

    // Header info
    BSPHeader header;


    // Dados carregados
    BSPLump lumps[kMaxLumps];
    BSPTexture* Textures{ nullptr };
    s32 NumTextures;

    BSPLightmap* LightMaps{ nullptr };
    s32 NumLightMaps;

    BSPVertex* Vertices{ nullptr };
    s32 NumVertices;

    BSPFace* Faces{ nullptr };
    s32 NumFaces;

    BSPModel* Models{ nullptr };
    s32 NumModels;

    BSPPlane* Planes{ nullptr };
    s32 NumPlanes;

    BSPNode* Nodes{ nullptr };
    s32 NumNodes;

    BSPLeaf* Leafs{ nullptr };
    s32 NumLeafs;

    s32* Indices{ nullptr };
    s32 NumIndices;

    s32* LeafFaces{ nullptr };
    s32 NumLeafFaces;

    s32* MeshVerts{ nullptr };
    s32 NumMeshVerts;

    BSPBrush* Brushes{ nullptr };
    s32 NumBrushes;

    s32 NumEntities;
    std::vector<u8> Entities;

    float lmgamma = { 1.0f };

    std::vector<BSPSurface> Surfaces;
    std::vector<BSPSurface> mergedSurfaces;
    //  std::vector<Image> images;
    std::vector<Mesh> meshes;

    void loadTexture(BinaryFile& file);
    void loadLightmap(BinaryFile& file);
    void loadVertex(BinaryFile& file);
    void loadFaces(BinaryFile& file);
    void loadIndex(BinaryFile& file);
    void LoadEntities(BinaryFile& file);
    void loadModels(BinaryFile& file);

    void BuildSurfaces();
    void MergeSurfacesByMaterial();
    void CreateMeshesFromMergedSurfaces();
 
    bool ProcessPolygonFace(const BSPFace& face, BSPSurface& surface);
    bool ProcessBezierPatch(const BSPFace& face);
    bool ProcessMeshFace(const BSPFace& face, BSPSurface& surface);
    bool ProcessBillboardFace(const BSPFace& face, BSPSurface& surface);
    bool ProcessBezierPatch(const BSPFace& face, BSPSurface& surface);

 
    float scale = {0.1f};
    Matrix transform;
      Shader shader;   
    std::vector<Texture2D> textures;
    std::vector<Texture2D> lightmaps;
    Texture2D default_texture;
    BoundingBox bounds;


    struct Vertex2TCoords
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv0;
        Vector2 uv1;
        Color color;
        Vertex2TCoords() { color = WHITE; }

        Vertex2TCoords(const BSPVertex& v)
        {
            position = v.vPosition;
            normal = v.vNormal;
            uv0 = v.vTextureCoord;
            uv1 = v.vLightmapCoord;
            color.r = v.color[0];
            color.g = v.color[1];
            color.b = v.color[2];
            color.a = v.color[3];
        }
        Vertex2TCoords(const Vertex2TCoords &dest) 
        {
            position = dest.position;
            normal = dest.normal;
            uv0 = dest.uv0;
            uv1 = dest.uv1;
            color = dest.color;
        }
        Vertex2TCoords& operator=(const Vertex2TCoords& dest)
        {
            position = dest.position;
            normal = dest.normal;
            uv0 = dest.uv0;
            uv1 = dest.uv1;
            color = dest.color;
            return *this;
        }


        void copy(Vertex2TCoords &dest)
        {
            dest.position.x = position.x;
            dest.position.y = position.z;
            dest.position.z = position.y;
            dest.normal = normal;
            dest.uv0 = uv0;
            dest.uv1 = uv1;
            dest.color = color;
        }
    };

    struct SBezier
		{
			BSPSurface *Patch;
			Vertex2TCoords control[9];

			void tesselate(s32 level,float scale);

            Vertex2TCoords Interpolated_quadratic(Vertex2TCoords p0,
                                                  Vertex2TCoords p1,
                                                  Vertex2TCoords p2, double f);

        private:
			s32	Level;

			std::vector<Vertex2TCoords> column[3];

		};
		SBezier Bezier;
        u32 view_count = { 0 };
  

public:
    bool loadFromFile(const std::string& filePath);
    void drawDebugSurfaces();
    void clear();
    void render(ViewFrustum& frustum);

    Shader* getShader() { return &shader; }
    void SetShader(Shader* shader) { this->shader = *shader; }

    const std::vector<BSPSurface>& getSurfaces() const { return mergedSurfaces; }

    u32 getViewCount() const { return  view_count; }
    BoundingBox getBounds() const { return bounds; }

    BSP();
    ~BSP();
};

#endif