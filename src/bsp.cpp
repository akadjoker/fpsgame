 
#include "bsp.hpp"
#include "frustum.hpp"
#include "binaryfile.hpp"

Texture2D LoadTextureFromName(const std::string& basePath,
                              const std::string& textureName)
{
    const char* extensions[] = {
        ".png", ".jpeg", ".jpg", ".tga", ".bmp",
    };

    for (const char* ext : extensions)
    {
        std::string fullPath = basePath + "/" + textureName + ext;
        if (FileExists(fullPath.c_str()))
        {
            // LogInfo( "Load   %s", fullPath.c_str());

            Texture2D tex = LoadTexture(fullPath.c_str());
            GenTextureMipmaps(&tex);
            SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
            return tex;
        }
    }

    Image fallback = GenImageChecked(128, 128, 10, 10, WHITE, BLACK);
    Texture2D tex = LoadTextureFromImage(fallback);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    UnloadImage(fallback);

    LogWarning("Load   %s to default", textureName.c_str());


    return tex;
}

Mesh createMeshFromSurface(const BSPSurface& surface, float scale = 1.0f)
{
    Mesh mesh = { 0 };

    int numVerts = surface.vertices.size();
    int numTris = surface.indices.size() / 3;

    mesh.vertexCount = numVerts;
    mesh.triangleCount = numTris;

    mesh.vertices = (float*)MemAlloc(numVerts * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(numVerts * 2 * sizeof(float));
    mesh.texcoords2 = (float*)MemAlloc(numVerts * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(numVerts * 3 * sizeof(float));
    mesh.colors = (unsigned char*)MemAlloc(numVerts * 4 * sizeof(unsigned char));
    mesh.indices =(unsigned short*)MemAlloc(numTris * 3 * sizeof(unsigned short));

    for (int i = 0; i < numVerts; ++i)
    {
        mesh.vertices[i * 3 + 0] = surface.vertices[i].x * scale;
        mesh.vertices[i * 3 + 1] = surface.vertices[i].y * scale;
        mesh.vertices[i * 3 + 2] = surface.vertices[i].z * scale;

        mesh.normals[i * 3 + 0] = surface.normals[i].x * scale;
        mesh.normals[i * 3 + 1] = surface.normals[i].y * scale;
        mesh.normals[i * 3 + 2] = surface.normals[i].z * scale;

        mesh.colors[i * 4 + 0] = surface.colors[i].r ;
        mesh.colors[i * 4 + 1] = surface.colors[i].g;
        mesh.colors[i * 4 + 2] = surface.colors[i].b;
        mesh.colors[i * 4 + 3] = surface.colors[i].a;

        mesh.texcoords[i * 2 + 0] = surface.uv0[i].x;
        mesh.texcoords[i * 2 + 1] = surface.uv0[i].y;


        mesh.texcoords2[i * 2 + 0] = surface.uv1[i].x;
        mesh.texcoords2[i * 2 + 1] = surface.uv1[i].y;
    }

    for (size_t i = 0; i < (surface.indices.size() / 3); ++i)
    {
        mesh.indices[i * 3 + 0] = surface.indices[i * 3 + 0];
        mesh.indices[i * 3 + 1] = surface.indices[i * 3 + 1];
        mesh.indices[i * 3 + 2] = surface.indices[i * 3 + 2];
    }

    UploadMesh(&mesh, false);
    return mesh;
}


void BSP::loadTexture(BinaryFile& file)
{

    NumTextures = lumps[kTextures].length / sizeof(BSPTexture);
    Textures = new BSPTexture[NumTextures];
    file.seek(lumps[kTextures].offset, SEEK_SET);
    file.readBytes(&Textures[0], lumps[kTextures].length);


    textures.reserve(NumTextures);

    for (int i = 0; i < NumTextures; ++i)
    {

        char path[1024];
        snprintf(path, sizeof(path), "%s", Textures[i].strName);
        textures[i] = LoadTextureFromName(".", path);
    }
}

void BSP::loadLightmap(BinaryFile& file)
{
    NumLightMaps = lumps[kLightmaps].length / sizeof(BSPLightmap);

    LightMaps = new BSPLightmap[NumLightMaps];
    file.seek(lumps[kLightmaps].offset, SEEK_SET);
    file.readBytes(&LightMaps[0], lumps[kLightmaps].length);


    LogInfo(" %d", NumLightMaps);
    lightmaps.reserve(NumLightMaps);
    for (int i = 0; i < NumLightMaps; ++i)
    {
        Image img = GenImageColor(128, 128, WHITE);

        for (int y = 0; y < 128; ++y)
        {
            for (int x = 0; x < 128; ++x)
            {
                u8 r = LightMaps[i].imageBits[y][x][0];
                u8 g = LightMaps[i].imageBits[y][x][1];
                u8 b = LightMaps[i].imageBits[y][x][2];

                ImageDrawPixel(&img, x, y, Color{ r, g, b, 255 });
            }
        }


        // ExportImage(img, TextFormat("lightmap%d.png", i));
        lightmaps[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
}

void BSP::loadVertex(BinaryFile& file)
{
    NumVertices = lumps[kVertices].length / sizeof(BSPVertex);

    Vertices = new BSPVertex[NumVertices];
    file.seek(lumps[kVertices].offset, SEEK_SET);
    file.readBytes(&Vertices[0], lumps[kVertices].length);
}

void BSP::loadFaces(BinaryFile& file)
{
    NumFaces = lumps[kFaces].length / sizeof(BSPFace);

    Faces = new BSPFace[NumFaces];
    file.seek(lumps[kFaces].offset, SEEK_SET);
    file.readBytes(&Faces[0], lumps[kFaces].length);
}

void BSP::loadIndex(BinaryFile& file)
{
    NumIndices = lumps[kIndices].length / sizeof(u16);

    Indices = new s32[NumIndices];
    file.seek(lumps[kIndices].offset, SEEK_SET);
    file.readBytes(&Indices[0], lumps[kIndices].length);
}

void BSP::LoadEntities(BinaryFile& file)
{
    NumEntities = lumps[kEntities].length / sizeof(u8);
    Entities.reserve(lumps[kEntities].length + 2);
    Entities[lumps[kEntities].length + 1] = '\0';

    file.seek(lumps[kEntities].offset, SEEK_SET);
    file.readBytes(&Entities[0], lumps[kEntities].length);

    if (Entities.empty()) return;

    
     char* buffer = new char[lumps[kEntities].length + 1];
     std::copy(Entities.begin(), Entities.end(), buffer);

     LogInfo("Entities: %s", buffer);
     
     SaveFileText("entities.txt", buffer);
     delete[] buffer;

    // for (auto i = 0; i < lumps[kEntities].length; i++)
    // {
    //     printf("%c", Entities[i]);
    // }
}

void BSP::loadModels(BinaryFile& file)
{
    NumModels = lumps[kModels].length / sizeof(BSPModel);

    Models = new BSPModel[NumModels];
    file.seek(lumps[kModels].offset, SEEK_SET);
    file.readBytes(&Models[0], lumps[kModels].length);
}


bool BSP::loadFromFile(const std::string& filePath)
{
    BinaryFile file;
    if (!file.open(filePath.c_str())) return false;

    file.readBytes(&header, sizeof(BSPHeader));

    LogInfo("BSP version: %d", header.version);
   // LogInfo("BSP ID: %d", header.strID);

    file.readBytes(&lumps, sizeof(BSPLump) * kMaxLumps);


    loadTexture(file);
    loadLightmap(file);
    loadVertex(file);
    loadFaces(file);
    loadIndex(file);
    LoadEntities(file);
    loadModels(file);

    BuildSurfaces();

    transform = MatrixIdentity();
    
 


    return true;
}

BSP::BSP()
 {
    
 }

BSP::~BSP() {}

void BSP::drawDebugSurfaces()
{

    LogInfo("Desenhando %d faces", Surfaces.size());
    // 2879


    rlPushMatrix();
    rlBegin(RL_LINES);
    rlColor3f(0.1f, 1.0f, 0.1f); // cor verde


    for (const BSPSurface& surface : Surfaces)
    {
        const std::vector<Vector3>& verts = surface.vertices;
        const std::vector<u16>& indices = surface.indices;

        //    LogInfo( "Desenhando %d faces", surface.textureID);

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            Vector3 v0 = verts[indices[i + 0]];
            Vector3 v1 = verts[indices[i + 1]];
            Vector3 v2 = verts[indices[i + 2]];

            rlVertex3f(v0.x, v0.y, v0.z);
            rlVertex3f(v1.x, v1.y, v1.z);

            rlVertex3f(v1.x, v1.y, v1.z);
            rlVertex3f(v2.x, v2.y, v2.z);

            rlVertex3f(v2.x, v2.y, v2.z);
            rlVertex3f(v0.x, v0.y, v0.z);
        }
    }

    rlEnd();
    rlPopMatrix();
}

void BSP::clear()
{
    for (u32 i = 0; i < Surfaces.size(); i++)
    {
       Surfaces[i].clear();
    }

    for (u32 i = 0; i < mergedSurfaces.size(); i++)
    {
        Surfaces[i].clear();
    }

    // for (auto& mesh : meshes)
    // {
    //     UnloadMesh(mesh);
    // }
    for (auto& texture : textures)
    {
        UnloadTexture(texture);
    }
    for (auto& lightmap : lightmaps)
    {
        UnloadTexture(lightmap);
    }

    delete[] Textures;
    Textures = 0;
    delete[] LightMaps;
    LightMaps = 0;
    delete[] Vertices;
    Vertices = 0;
    delete[] Faces;
    Faces = 0;
    delete[] Models;
    Models = 0;
    delete[] Planes;
    Planes = 0;
    delete[] Nodes;
    Nodes = 0;
    delete[] Leafs;
    Leafs = 0;
    delete[] LeafFaces;
    LeafFaces = 0;
    delete[] MeshVerts;
    MeshVerts = 0;
    delete[] Brushes;
    Brushes = 0;
    delete[] Indices;
    Indices = 0;
}



void BSP::BuildSurfaces()
{
    Surfaces.clear();
    Surfaces.reserve(NumFaces);



 

    // Primeiro: construir todas as superfícies individuais
    for (int i = 0; i < NumFaces; i++)
    {
        BSPFace face = Faces[i];


        s32 textureID = face.textureID;

        if (textureID < 0 || textureID >= NumTextures)
        {
            LogWarning("Invalid textureID %d for face %d", textureID, i);
            continue;
        }

        Surfaces.push_back(BSPSurface());
        BSPSurface& surface = Surfaces.back();
        surface.textureID = textureID;

        if (face.lightmapID >= 0 && face.lightmapID < NumLightMaps)
        {
            surface.lightmapID = face.lightmapID;
        }
        else
        {
            surface.lightmapID = -1;
        }


        // Carregar vértices
        
            for (s32 v = face.startVertIndex;v < face.startVertIndex + face.numOfVerts; v++)
            {
                if (v >= NumVertices)
                {
                    LogWarning("Vertex index %d out of bounds", v);
                    continue;
                }

                BSPVertex vertex = Vertices[v];
                surface.vertices.push_back((Vector3){ vertex.vPosition.x * scale,
                                                      vertex.vPosition.z * scale,
                                                      vertex.vPosition.y * scale });
                surface.normals.push_back((Vector3){vertex.vNormal.x * scale, vertex.vNormal.z * scale, vertex.vNormal.y* scale});
                surface.colors.push_back(( Color){vertex.color[0] , vertex.color[1], vertex.color[2], vertex.color[3]});
              //  surface.colors.push_back(WHITE);

               // LogInfo("Color: %d %d %d %d", vertex.color[0], vertex.color[1], vertex.color[2], vertex.color[3]);


                surface.uv0.push_back(vertex.vTextureCoord);
                surface.uv1.push_back(vertex.vLightmapCoord);
            }
        

        if (face.type == 1) // Polygon
        {
             ProcessPolygonFace(face, surface);
        }
        else if (face.type == 2) // Patch
        {
            ProcessBezierPatch(face, surface);
        }
        else if (face.type == 3) // Mesh
        {
            ProcessMeshFace(face, surface);
        }
        else
        {
            LogInfo("Face type %d not supported, treating as simple mesh",face.type);

        }
     //  surface.init();
    }
    
    // for (u32 i = 0; i < Surfaces.size(); i++)
    // {
    //     Surfaces[i].init();
    // }

    // for (const BSPSurface& surface : Surfaces)
    // { 
    //     Mesh mesh = createMeshFromSurface(surface, 0.01f);
    //     meshes.push_back(mesh);
    // }
    
    MergeSurfacesByMaterial();


    Vector3 min = mergedSurfaces[0].bounds.min;
    Vector3 max = mergedSurfaces[0].bounds.max;

    for (size_t i = 1; i < mergedSurfaces.size(); ++i)
    {
        const BoundingBox& b = mergedSurfaces[i].bounds;

        min.x = fminf(min.x, b.min.x);
        min.y = fminf(min.y, b.min.y);
        min.z = fminf(min.z, b.min.z);

        max.x = fmaxf(max.x, b.max.x);
        max.y = fmaxf(max.y, b.max.y);
        max.z = fmaxf(max.z, b.max.z);
    }
    bounds.min = min;
    bounds.max = max;
    
    
  //      CreateMeshesFromMergedSurfaces();
  //  Surfaces.clear();
}

void BSP::MergeSurfacesByMaterial()
{
    if (Surfaces.empty()) return;


    

    // Criar um mapa para agrupar por textureID e lightmapID
    std::map<std::pair<int, int>, std::vector<int>> materialGroups;

    // Agrupar superfícies por material
    for (size_t i = 0; i < Surfaces.size(); i++)
    {
        std::pair<int, int> materialKey = { Surfaces[i].textureID,
                                            Surfaces[i].lightmapID };
        materialGroups[materialKey].push_back(i);
    }

    // Limpar superfícies merged anteriores
    mergedSurfaces.clear();
    mergedSurfaces.reserve(materialGroups.size());

    // Merge para cada grupo de material
    for (const auto& group : materialGroups)
    {
        BSPSurface mergedSurface;
        mergedSurface.textureID = group.first.first; // textureID
        mergedSurface.lightmapID = group.first.second; // lightmapID

        // Calcular tamanho total necessário para otimização
        size_t totalVertices = 0;
        size_t totalIndices = 0;
        for (int surfaceIndex : group.second)
        {
            totalVertices += Surfaces[surfaceIndex].vertices.size();
            totalIndices += Surfaces[surfaceIndex].indices.size();
        }

        // Reservar espaço
        mergedSurface.vertices.reserve(totalVertices);
        mergedSurface.normals.reserve(totalVertices);
        mergedSurface.uv0.reserve(totalVertices);
        mergedSurface.uv1.reserve(totalVertices);
        mergedSurface.indices.reserve(totalIndices);
        mergedSurface.colors.reserve(totalVertices);

        // Merge todas as superfícies do grupo
        for (int surfaceIndex : group.second)
        {
            const BSPSurface& surface = Surfaces[surfaceIndex];
            int vertexOffset = mergedSurface.vertices.size();

            // Adicionar vértices
            for (size_t i = 0; i < surface.vertices.size(); i++)
            {
                mergedSurface.vertices.push_back(surface.vertices[i]);
                mergedSurface.normals.push_back(surface.normals[i]);
                mergedSurface.uv0.push_back(surface.uv0[i]);
                mergedSurface.uv1.push_back(surface.uv1[i]);
                mergedSurface.colors.push_back(surface.colors[i]);
            }

          
            for (size_t i = 0; i < surface.indices.size(); i++)
            {
                mergedSurface.indices.push_back(surface.indices[i]+ vertexOffset);
            }
        }

        mergedSurface.init();
        mergedSurfaces.push_back(mergedSurface);

        LogInfo("Merged %d surfaces with texture %d, lightmap %d (%d "
                "vertices, %d indices)",
                (int)group.second.size(), mergedSurface.textureID,
                mergedSurface.lightmapID, (int)mergedSurface.vertices.size(),
                (int)mergedSurface.indices.size());
    }
}




// Tipo 1: Polygon - face plana triangulada
bool BSP::ProcessPolygonFace(const BSPFace& face, BSPSurface& surface)
{
    if (face.numOfIndices % 3 != 0)
    {
        LogWarning("Polygon face has invalid number of indices: %d",
                   face.numOfIndices);
        return false;
    }

    for (s32 j = 0; j < face.numOfIndices; j += 3)
    {
        if (face.startIndex + j + 2 >= NumIndices)
        {
            LogWarning("Polygon index out of bounds");
            break;
        }

        surface.indices.push_back(Indices[face.startIndex + j + 0]);
        surface.indices.push_back(Indices[face.startIndex + j + 1]);
        surface.indices.push_back(Indices[face.startIndex + j + 2]);
    }

    return true;
}
 
 
// Tipo 3: Mesh -
bool BSP::ProcessMeshFace(const BSPFace& face, BSPSurface& surface)
{
    if (face.numOfIndices % 3 != 0)
    {
        LogWarning("Mesh face has invalid number of indices: %d",
                   face.numOfIndices);
        return false;
    }

    // Para meshes, os índices são absolutos (não relativos à face)
    for (s32 j = 0; j < face.numOfIndices; j += 3)
    {
        if (face.startIndex + j + 2 >= NumIndices)
        {
            LogWarning("Mesh index out of bounds");
            break;
        }

        // Ajustar índices para serem relativos aos vértices desta superfície
        int idx0 = Indices[face.startIndex + j + 0] - face.startVertIndex;
        int idx1 = Indices[face.startIndex + j + 1] - face.startVertIndex;
        int idx2 = Indices[face.startIndex + j + 2] - face.startVertIndex;

        // Validar se os índices estão dentro do range
        if (idx0 >= 0 && idx0 < face.numOfVerts && idx1 >= 0
            && idx1 < face.numOfVerts && idx2 >= 0 && idx2 < face.numOfVerts)
        {
            surface.indices.push_back(idx0);
            surface.indices.push_back(idx1);
            surface.indices.push_back(idx2);
        }
        else
        {
            LogWarning("Mesh indices out of vertex range");
        }
    }

    return true;
}

// Tipo 4: Billboard - quad sempre virado para a câmera
bool BSP::ProcessBillboardFace(const BSPFace& face, BSPSurface& surface)
{
    if (face.numOfVerts != 4)
    {
        LogWarning("Billboard must have exactly 4 vertices, has %d",
                   face.numOfVerts);
        return false;
    }

    // Billboard é um quad, precisa de 2 triângulos (6 índices)
    // Assumindo que os vértices estão em ordem: bottom-left, bottom-right,
    // top-right, top-left
    surface.indices.push_back(0); // bottom-left
    surface.indices.push_back(1); // bottom-right
    surface.indices.push_back(2); // top-right

    surface.indices.push_back(0); // bottom-left
    surface.indices.push_back(2); // top-right
    surface.indices.push_back(3); // top-left

    // Marcar como billboard para tratamento especial no shader
    // surface.isBillboard = true;

    return true;
}


bool BSP::ProcessBezierPatch(const BSPFace& face, BSPSurface& surface)
{
 


    int controlWidth = face.size[0];
    int controlHeight = face.size[1];

    if (controlWidth == 0 || controlHeight == 0) return false;

    int biquadWidth = (controlWidth - 1) / 2;
    int biquadHeight = (controlHeight - 1) / 2;

    std::vector<Vertex2TCoords> controlPoint;
    controlPoint.reserve(controlWidth * controlHeight);

    for (int i = 0; i < controlWidth * controlHeight; ++i)
    {
        controlPoint.push_back(Vertices[face.startVertIndex + i]);
    }

    Bezier.Patch = new BSPSurface();

    for (int j = 0; j < biquadHeight; ++j)
    {
        for (int k = 0; k < biquadWidth; ++k)
        {
           const s32 inx = j*controlWidth*2 + k*2;

			// setup bezier control points for this patch
			Bezier.control[0] = controlPoint[ inx + 0];
			Bezier.control[1] = controlPoint[ inx + 1];
			Bezier.control[2] = controlPoint[ inx + 2];
			Bezier.control[3] = controlPoint[ inx + controlWidth + 0 ];
			Bezier.control[4] = controlPoint[ inx + controlWidth + 1 ];
			Bezier.control[5] = controlPoint[ inx + controlWidth + 2 ];
			Bezier.control[6] = controlPoint[ inx + controlWidth * 2 + 0];
			Bezier.control[7] = controlPoint[ inx + controlWidth * 2 + 1];
			Bezier.control[8] = controlPoint[ inx + controlWidth * 2 + 2];

            Bezier.tesselate( 5,scale );
        }
    }

    const u32 bsize = Bezier.Patch->vertices.size();
	const u32 msize = surface.vertices.size();

    surface.vertices.reserve(msize + bsize);
    surface.normals.reserve(msize + bsize);
    surface.uv0.reserve(msize + bsize);
    surface.uv1.reserve(msize + bsize);
    surface.colors.reserve(msize + bsize);
    

    for (u32 i = 0; i != bsize; ++i)
    {
        surface.vertices.push_back(Bezier.Patch->vertices[i]);
        surface.normals.push_back(Bezier.Patch->normals[i]);
        surface.uv0.push_back(Bezier.Patch->uv0[i]);
        surface.uv1.push_back(Bezier.Patch->uv1[i]);
        surface.colors.push_back(Bezier.Patch->colors[i]);
    }

    surface.indices.reserve(surface.indices.size() + Bezier.Patch->indices.size());

    for (u32 i = 0; i != Bezier.Patch->indices.size(); ++i)
    {
        surface.indices.push_back(msize + Bezier.Patch->indices[i]);
    }

    delete Bezier.Patch;
    Bezier.Patch = nullptr;

    return true;
} 

void BSP::SBezier::tesselate( s32 level ,float scale)
{
	//Calculate how many vertices across/down there are
	s32 j, k;

	column[0].reserve( level + 1 );
	column[1].reserve( level + 1 );
	column[2].reserve( level + 1 );

	const double w = 0.0 + (1.0 / (double) level );

	//Tesselate along the columns
	for( j = 0; j <= level; ++j)
	{
		const double f = w * (double) j;

		column[0][j] =Interpolated_quadratic(control[0],control[3], control[6], f );
		column[1][j] =Interpolated_quadratic(control[1],control[4], control[7], f );
		column[2][j] =Interpolated_quadratic(control[2],control[5], control[8], f );
	}

	const u32 idx = Patch->vertices.size();
	Patch->vertices.reserve(idx+level*level);
    Patch->normals.reserve(idx+level*level);
    Patch->uv0.reserve(idx+level*level);
    Patch->uv1.reserve(idx+level*level);
    Patch->colors.reserve(idx+level*level);
	Vertex2TCoords v;
	Vertex2TCoords f;
	for( j = 0; j <= level; ++j)
	{
		for( k = 0; k <= level; ++k)
		{
			f = Interpolated_quadratic(column[0][j],column[1][j], column[2][j], w * (double) k);
			f.copy( v );

            Vector3 pos;

            pos.x = v.position.x * scale;
            pos.y = v.position.y * scale;
            pos.z = v.position.z * scale;

            Vector3 norm;

            norm.x = v.normal.x * scale;
            norm.y = v.normal.y * scale;
            norm.z = v.normal.z * scale;

			Patch->vertices.push_back( pos );
            Patch->normals.push_back( norm );
            Patch->uv0.push_back( v.uv0 );
            Patch->uv1.push_back( v.uv1 );
            Patch->colors.push_back( v.color );

		}
	}

	Patch->indices.reserve(Patch->indices.size()+6*level*level);
 
	for( j = 0; j < level; ++j)
	{
		for( k = 0; k < level; ++k)
		{
			const s32 inx = idx + ( k * ( level + 1 ) ) + j;

			Patch->indices.push_back( inx + 0 );
			Patch->indices.push_back( inx + (level + 1 ) + 0 );
			Patch->indices.push_back( inx + (level + 1 ) + 1 );

			Patch->indices.push_back( inx + 0 );
			Patch->indices.push_back( inx + (level + 1 ) + 1 );
			Patch->indices.push_back( inx + 1 );
		}
	}
}

inline double Clamp(double val, double min, double max)
{
    return val < min ? min : (val > max ? max : val);
}

inline float GetValue(unsigned char val)
{
    return val / 255.0f;
}

inline unsigned char ToByte(float val)
{
    return (unsigned char)Clamp(val * 255.0f, 0.0f, 255.0f);
}

static Color Interpolated_color_quadratic(const Color& c0, const Color& c1, const Color& c2, float d)
{
    d = Clamp(d, 0.f, 1.f);
    const float inv = 1.f - d;
    const float mul0 = inv * inv;
    const float mul1 = 2.f * d * inv;
    const float mul2 = d * d;

    float a = GetValue(c0.a) * mul0 + GetValue(c1.a) * mul1 + GetValue(c2.a) * mul2;
    float r = GetValue(c0.r) * mul0 + GetValue(c1.r) * mul1 + GetValue(c2.r) * mul2;
    float g = GetValue(c0.g) * mul0 + GetValue(c1.g) * mul1 + GetValue(c2.g) * mul2;
    float b = GetValue(c0.b) * mul0 + GetValue(c1.b) * mul1 + GetValue(c2.b) * mul2;

    Color result;
    result.a =  ToByte(a);
    result.r = ToByte(r);
    result.g = ToByte(g);
    result.b = ToByte(b);

    return result;
}

static Vector3 Interpolated_vec3_quadratic(const Vector3& v1, const Vector3& v2,
                                    const Vector3& v3, double d)
{

    const double inv = 1.0f - d;
    const double mul0 = inv * inv;
    const double mul1 = 2.0f * d * inv;
    const double mul2 = d * d;

    Vector3 v;
    v.x = (double)(v1.x * mul0 + v2.x * mul1 + v3.x * mul2);
    v.y = (double)(v1.y * mul0 + v2.y * mul1 + v3.y * mul2);
    v.z = (double)(v1.z * mul0 + v2.z * mul1 + v3.z * mul2);

    return v;
}

static Vector2 Interpolated_vec2_quadratic(const Vector2& v1, const Vector2& v2,
                                    const Vector2& v3, double d)
{

    const double inv = 1.0f - d;
    const double mul0 = inv * inv;
    const double mul1 = 2.0f * d * inv;
    const double mul2 = d * d;

    Vector2 v;
    v.x = (double)(v1.x * mul0 + v2.x * mul1 + v3.x * mul2);
    v.y = (double)(v1.y * mul0 + v2.y * mul1 + v3.y * mul2);

    return v;
}

 

BSP::Vertex2TCoords BSP::SBezier::Interpolated_quadratic(BSP::Vertex2TCoords p0,
                                                    BSP::Vertex2TCoords p1,
                                                    BSP::Vertex2TCoords p2, double f)
{
    BSP::Vertex2TCoords result;
    result.position = Interpolated_vec3_quadratic(p0.position, p1.position, p2.position, f);
    result.normal = Interpolated_vec3_quadratic(p0.normal, p1.normal, p2.normal, f);
    result.uv0 = Interpolated_vec2_quadratic(p0.uv0, p1.uv0, p2.uv0, f);
    result.uv1 = Interpolated_vec2_quadratic(p0.uv1, p1.uv1, p2.uv1, f);
    result.color =  Interpolated_color_quadratic(p0.color, p1.color, p2.color, f);
    return result;
}

void BSPSurface::createCube()
{
    // 8 vértices do cubo (1x1x1 centrado na origem)
    vertices = {
        // Face frontal
        {-0.5f, -0.5f,  0.5f}, // 0 - inferior esquerdo
        { 0.5f, -0.5f,  0.5f}, // 1 - inferior direito
        { 0.5f,  0.5f,  0.5f}, // 2 - superior direito
        {-0.5f,  0.5f,  0.5f}, // 3 - superior esquerdo
        
        // Face traseira
        {-0.5f, -0.5f, -0.5f}, // 4 - inferior esquerdo
        { 0.5f, -0.5f, -0.5f}, // 5 - inferior direito
        { 0.5f,  0.5f, -0.5f}, // 6 - superior direito
        {-0.5f,  0.5f, -0.5f}, // 7 - superior esquerdo
        
        // Face direita
        { 0.5f, -0.5f,  0.5f}, // 8
        { 0.5f, -0.5f, -0.5f}, // 9
        { 0.5f,  0.5f, -0.5f}, // 10
        { 0.5f,  0.5f,  0.5f}, // 11
        
        // Face esquerda
        {-0.5f, -0.5f, -0.5f}, // 12
        {-0.5f, -0.5f,  0.5f}, // 13
        {-0.5f,  0.5f,  0.5f}, // 14
        {-0.5f,  0.5f, -0.5f}, // 15
        
        // Face superior
        {-0.5f,  0.5f,  0.5f}, // 16
        { 0.5f,  0.5f,  0.5f}, // 17
        { 0.5f,  0.5f, -0.5f}, // 18
        {-0.5f,  0.5f, -0.5f}, // 19
        
        // Face inferior
        {-0.5f, -0.5f, -0.5f}, // 20
        { 0.5f, -0.5f, -0.5f}, // 21
        { 0.5f, -0.5f,  0.5f}, // 22
        {-0.5f, -0.5f,  0.5f}  // 23
    };
    
    // Coordenadas UV (0-1 para cada face)
    uv0 = {
        // Face frontal
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Face traseira  
        {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
        // Face direita
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Face esquerda
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Face superior
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Face inferior
        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
    };
    
    uv1 = uv0; // Mesmas coordenadas para lightmap
    
    // Cores diferentes para cada face (para debug visual)
    colors = {
        // Face frontal (vermelho)
        {255, 0, 0, 255}, {255, 0, 0, 255}, {255, 0, 0, 255}, {255, 0, 0, 255},
        // Face traseira (verde)
        {0, 255, 0, 255}, {0, 255, 0, 255}, {0, 255, 0, 255}, {0, 255, 0, 255},
        // Face direita (azul)
        {0, 0, 255, 255}, {0, 0, 255, 255}, {0, 0, 255, 255}, {0, 0, 255, 255},
        // Face esquerda (amarelo)
        {255, 255, 0, 255}, {255, 255, 0, 255}, {255, 255, 0, 255}, {255, 255, 0, 255},
        // Face superior (magenta)
        {255, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 255, 255},
        // Face inferior (ciano)
        {0, 255, 255, 255}, {0, 255, 255, 255}, {0, 255, 255, 255}, {0, 255, 255, 255}
    };
    
    // Normais para cada face
    normals = {
        // Face frontal (Z+)
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        // Face traseira (Z-)
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        // Face direita (X+)
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        // Face esquerda (X-)
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        // Face superior (Y+)
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        // Face inferior (Y-)
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
    };
    
    // Índices para formar os triângulos (2 triângulos por face)
    indices = {
        // Face frontal
        0, 1, 2,   2, 3, 0,
        // Face traseira
        4, 5, 6,   6, 7, 4,
        // Face direita
        8, 9, 10,  10, 11, 8,
        // Face esquerda
        12, 13, 14, 14, 15, 12,
        // Face superior
        16, 17, 18, 18, 19, 16,
        // Face inferior
        20, 21, 22, 22, 23, 20
    };
}

void BSPSurface::init() 
{
    bool dynamic = false;


    vertexCount  = vertices.size();
    triangleCount = indices.size() /3;

    
 

    vaoId = rlLoadVertexArray();
    rlEnableVertexArray(vaoId);
    
    
    // Vértices
    vboId[0] = rlLoadVertexBuffer(vertices.data(), vertexCount  * sizeof(Vector3), dynamic);
    rlEnableVertexBuffer(vboId[0]); 
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    
    // UV principal
    vboId[1] = rlLoadVertexBuffer(uv0.data(), vertexCount  * sizeof(Vector2), dynamic);
    rlEnableVertexBuffer(vboId[1]);
    rlEnableVertexAttribute(1);
    rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
    
    // UV lightmap
    vboId[2] = rlLoadVertexBuffer(uv1.data(), vertexCount  * sizeof(Vector2), dynamic);
    rlEnableVertexBuffer(vboId[2]);
    rlEnableVertexAttribute(2);
    rlSetVertexAttribute(2, 2, RL_FLOAT, 0, 0, 0);
    
    // Cor por vértice
    vboId[3] = rlLoadVertexBuffer(colors.data(), vertexCount * 4 * sizeof(unsigned char), dynamic);
    rlEnableVertexBuffer(vboId[3]);
    rlEnableVertexAttribute(3);
    rlSetVertexAttribute(3, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
    
    
    vboId[4] = rlLoadVertexBufferElement(indices.data(), triangleCount *3  * sizeof(u16), dynamic);
    rlEnableVertexBufferElement(vboId[4]);
    
 //  LogInfo("VAO: [ID %i] Mesh uploaded successfully (%i tris, %i verts)", vaoId,triangleCount*3, vertexCount);
    
    rlDisableVertexArray();
    updateBounds();
}

void BSPSurface::clear() 
{
    if (vaoId == 0) return;
    rlUnloadVertexArray(vaoId);
    rlUnloadVertexBuffer(vboId[0]);
    rlUnloadVertexBuffer(vboId[1]);
    rlUnloadVertexBuffer(vboId[2]);
    rlUnloadVertexBuffer(vboId[3]);
    rlUnloadVertexBuffer(vboId[4]);
    vaoId = 0;
    vboId[0] = vboId[1] = vboId[2] = vboId[3] = vboId[4] = 0;
}
void BSPSurface::update() 
{

}

 



void BSPSurface::render()  
{
    if (vaoId == 0 || triangleCount == 0 || vertexCount == 0) return;
     rlEnableVertexArray(vaoId);
     rlDrawVertexArrayElements(0, triangleCount*3 , 0);

}

void BSP::render(ViewFrustum& frustum, Shader &shader)
{

    view_count = 0;
    
    Matrix matView = rlGetMatrixModelview();
    Matrix matProjection = rlGetMatrixProjection();
    Matrix matModelView = MatrixMultiply(transform, matView);
    Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

    rlSetUniformMatrix(1, matModelViewProjection);

//  for (u32 i = 0; i < Surfaces.size(); i++)
//     {
//         const BSPSurface& surface = Surfaces[i];
//         rlActiveTextureSlot(0);
//         rlEnableTexture(textures[surface.textureID].id);
//         rlActiveTextureSlot(1);
//         rlEnableTexture(lightmaps[surface.lightmapID].id);
//         Surfaces[i].render();
//     }

    for (u32 i = 0; i < mergedSurfaces.size(); i++)
    {
        if (!frustum.isBoxInside(mergedSurfaces[i].bounds)) continue;
        const BSPSurface& surface = mergedSurfaces[i];
        rlActiveTextureSlot(0);
        rlEnableTexture(textures[surface.textureID].id);
        if (surface.lightmapID != -1 && surface.lightmapID < (s32)lightmaps.size())
        {
            rlActiveTextureSlot(1);
            rlEnableTexture(lightmaps[surface.lightmapID].id);
        } else 
        {
            rlActiveTextureSlot(1);
            rlDisableTexture();
        }
        mergedSurfaces[i].render();
        view_count++;
    }


    rlDisableVertexArray();
    rlDisableTexture();
    rlDisableShader();
    // rlSetMatrixModelview(matView);
    // rlSetMatrixProjection(matProjection);
  //  DrawBoundingBox(bounds, LIME);


    //  for (u32 i = 0; i < mergedSurfaces.size(); i++)
    // {
    //     const BSPSurface& surface = mergedSurfaces[i];
       
    //     DrawBoundingBox(mergedSurfaces[i].bounds, Color{ 255, 0, 0, 255 });
    // }

}

void BSPSurface::updateBounds() 
{
    if (vertices.empty())
    {
        bounds.min = (Vector3){ 0 };
        bounds.max = (Vector3){ 0 };
        return;
    }

    bounds.min = vertices[0];
    bounds.max = vertices[0];

    for (size_t i = 1; i < vertices.size(); ++i)
    {
        Vector3 v = vertices[i];
        if (v.x < bounds.min.x) bounds.min.x = v.x;
        if (v.y < bounds.min.y) bounds.min.y = v.y;
        if (v.z < bounds.min.z) bounds.min.z = v.z;
        if (v.x > bounds.max.x) bounds.max.x = v.x;
        if (v.y > bounds.max.y) bounds.max.y = v.y;
        if (v.z > bounds.max.z) bounds.max.z = v.z;
    }
}
