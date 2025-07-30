 
#include "md3.hpp"
#include "animation.hpp"

#define MAT(mat, row, col) (((float*)&mat)[col * 4 + row])


 
 

Quaternion CreateFromMatrix(float *pTheMatrix, int rowColumnCount)
{
    Quaternion q = {0, 0, 0, 1}; // Inicializar como identidade
    
    if(!pTheMatrix || ((rowColumnCount != 3) && (rowColumnCount != 4))) 
        return q;

    float *pMatrix = pTheMatrix;
    float m4x4[16] = {0};

    // Converter 3x3 para 4x4  
    if(rowColumnCount == 3)
    {
        //  Transpor a matriz 3x3 de row-major (MD3) para column-major (OpenGL)
        // MD3 row-major:     OpenGL column-major:
        // [0 1 2]            [0 3 6]
        // [3 4 5]    --->    [1 4 7]  
        // [6 7 8]            [2 5 8]
        
        m4x4[0]  = pTheMatrix[0];   m4x4[4]  = pTheMatrix[1];   m4x4[8]  = pTheMatrix[2];   m4x4[12] = 0;
        m4x4[1]  = pTheMatrix[3];   m4x4[5]  = pTheMatrix[4];   m4x4[9]  = pTheMatrix[5];   m4x4[13] = 0;
        m4x4[2]  = pTheMatrix[6];   m4x4[6]  = pTheMatrix[7];   m4x4[10] = pTheMatrix[8];   m4x4[14] = 0;
        m4x4[3]  = 0;               m4x4[7]  = 0;               m4x4[11] = 0;               m4x4[15] = 1;

        pMatrix = &m4x4[0];
    }

 
    float trace = pMatrix[0] + pMatrix[5] + pMatrix[10] + 1.0f;
    float scale = 0.0f;

    if(trace > 0.00000001f)
    {
        scale = sqrtf(trace) * 2.0f;
        q.x = (pMatrix[9] - pMatrix[6]) / scale;
        q.y = (pMatrix[2] - pMatrix[8]) / scale;
        q.z = (pMatrix[4] - pMatrix[1]) / scale;
        q.w = 0.25f * scale;
    }
    else 
    {
        if (pMatrix[0] > pMatrix[5] && pMatrix[0] > pMatrix[10])  
        {   
            scale = sqrtf(1.0f + pMatrix[0] - pMatrix[5] - pMatrix[10]) * 2.0f;
            q.x = 0.25f * scale;
            q.y = (pMatrix[4] + pMatrix[1]) / scale;
            q.z = (pMatrix[2] + pMatrix[8]) / scale;
            q.w = (pMatrix[9] - pMatrix[6]) / scale; 
        } 
        else if (pMatrix[5] > pMatrix[10]) 
        {
            scale = sqrtf(1.0f + pMatrix[5] - pMatrix[0] - pMatrix[10]) * 2.0f;
            q.x = (pMatrix[4] + pMatrix[1]) / scale;
            q.y = 0.25f * scale;
            q.z = (pMatrix[9] + pMatrix[6]) / scale;
            q.w = (pMatrix[2] - pMatrix[8]) / scale;
        } 
        else 
        {   
            scale = sqrtf(1.0f + pMatrix[10] - pMatrix[0] - pMatrix[5]) * 2.0f;
            q.x = (pMatrix[2] + pMatrix[8]) / scale;
            q.y = (pMatrix[9] + pMatrix[6]) / scale;
            q.z = 0.25f * scale;
            q.w = (pMatrix[4] - pMatrix[1]) / scale;
        }
    }
    
 
    float length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if(length > 0.00000001f) {
        q.x /= length;
        q.y /= length;
        q.z /= length;
        q.w /= length;
    }
    
    return q;
}

Matrix BuildTagMatrix(tMd3Tag& tagA, tMd3Tag& tagB, float t)
{
    // Interpolação da posição
    Vector3 position = Vector3Lerp(tagA.position, tagB.position, t);

    // Criar quaternions a partir das matrizes 3x3 do MD3 (já transpostas)
    Quaternion qA = CreateFromMatrix(tagA.rotation, 3);
    Quaternion qB = CreateFromMatrix(tagB.rotation, 3);

 
    Quaternion qInterpolated = QuaternionSlerp(qA, qB, t);

 
    Matrix result = QuaternionToMatrix(qInterpolated);

    // Colocar a posição nos campos corretos para column-major
    result.m12 = position.x;  // m[0][3] - posição X
    result.m13 = position.y;  // m[1][3] - posição Y  
    result.m14 = position.z;  // m[2][3] - posição Z

    return result;
}
  
 
 

LoadMD3::LoadMD3()  
{
    m_animator= new MD3Animator(this);
    position = (Vector3){0.0f, 0.0f, 0.0f};
    rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    trasform = MatrixIdentity();
}

LoadMD3::~LoadMD3() 
{
		 
   
    delete m_animator;
  
    for (u32 i = 0; i < m_bones.size(); i++)
    {
        delete m_bones[i];
    }
    m_bones.clear();

}






bool LoadMD3::Load(const char* szFileName,     float scale) 
{
    BinaryFile file;
    if (!file.open(szFileName)) return false;

    file.readBytes(&m_Header, sizeof(tMd3Header));

    numFrames = m_Header.numFrames;
  

 
	char *ID = m_Header.fileID;

	 
	if((ID[0] != 'I' || ID[1] != 'D' || ID[2] != 'P' || ID[3] != '3') || m_Header.version != 15)
	{
		LogError("Bad MD3 file: %s", szFileName);
		return false;
	}
	
	 int i = 0;

 
	tMd3Bone *pBones = new tMd3Bone [m_Header.numFrames];
    file.readBytes(&pBones[0], sizeof(tMd3Bone) * m_Header.numFrames);

 



      numOfTags = m_Header.numTags;
   
      for (i = 0; i < m_Header.numFrames * m_Header.numTags; i++)
      {
          tMd3Tag tag;
          //  Vector3 	axis[3];
		  file.readBytes(&tag.strName[0], 64);
          tag.position.x = file.readFloat() * scale;
          tag.position.y = file.readFloat() * scale;
          tag.position.z = file.readFloat() * scale;
          
          tag.axes[0].x = file.readFloat();
          tag.axes[0].y = file.readFloat();
          tag.axes[0].z = file.readFloat();
          
          tag.axes[1].x = file.readFloat();
          tag.axes[1].y = file.readFloat();
          tag.axes[1].z = file.readFloat();
          
          tag.axes[2].x = file.readFloat();
          tag.axes[2].y = file.readFloat();
          tag.axes[2].z = file.readFloat();
   
          

          
          

           
   
 

          m_tags.push_back(tag);
      }


   

    //m_links.reserve(m_Header.numTags);

    for (i=0; i < m_Header.numTags; i++)
    {
        LogInfo("Tag: %s", m_tags[i].strName);
        Bone3D bone;
        bone.link = nullptr;
        bone.id = i;
        strcpy(bone.name, m_tags[i].strName);
        m_links.push_back(bone);
      
    
     }
 
    
 



    delete [] pBones;
	
	 
	
	long meshOffset = file.ftell();

	 
	tMd3MeshInfo meshHeader;

 
	for (i = 0; i < m_Header.numMeshes; i++)
	{
		tMD3Surface surface;
		
        file.seek(meshOffset, SEEK_SET);
        file.readBytes(&meshHeader, sizeof(tMd3MeshInfo));
		
        int numOfVerts = meshHeader.numVertices;
        int numOfTris = meshHeader.numTriangles;
        int numOfSkins = meshHeader.numSkins;
        int numOfFrames = meshHeader.numMeshFrames;
        int numOfFaces = meshHeader.numTriangles;
		surface.numFrames = numOfFrames;

        LogInfo("Name: %s", meshHeader.strName);
        LogInfo("Num of verts: %d", numOfVerts);
        LogInfo("Num of tris: %d", numOfTris);
        LogInfo("Num of skins: %d", numOfSkins);
        LogInfo("Num of frames: %d", numOfFrames);
        LogInfo("Num of faces: %d", numOfFaces);
		
    	surface.name = meshHeader.strName;
		surface.numTriangles = meshHeader.numTriangles;



		tMd3Skin				*m_pSkins      = new tMd3Skin [meshHeader.numSkins];
 
	
	

	    file.readBytes(&m_pSkins[0], sizeof(tMd3Skin) * meshHeader.numSkins);
		
		file.seek(meshOffset + meshHeader.triStart, SEEK_SET);



		for (int i = 0; i < meshHeader.numTriangles; i++)
		{
			int v1 = file.readInt();
			int v2 = file.readInt();
			int v3 = file.readInt();
            surface.indices.push_back(v3);
            surface.indices.push_back(v2);
            surface.indices.push_back(v1);
        }

 	 
 
        
        file.seek(meshOffset + meshHeader.vertexStart, SEEK_SET);

		
		for (int i = 0; i < meshHeader.numMeshFrames * meshHeader.numVertices; i++)
		{
			float x = static_cast<float>(file.readShort() ) / 64.0f;
			float y = static_cast<float>(file.readShort() )  / 64.0f;
			float z = static_cast<float>(file.readShort() ) / 64.0f;
			file.readByte();
			file.readByte();
	
			surface.vertices.push_back((Vector3){ x *scale ,y *scale ,z *scale });
		}

        surface.bounds.min = surface.vertices[0];
        surface.bounds.max = surface.vertices[0];
        bounds.min = {FLT_MAX, FLT_MAX, FLT_MAX};
        bounds.max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};            

        file.seek(meshOffset + meshHeader.uvStart, SEEK_SET);
		for (int i = 0; i < meshHeader.numVertices; i++)
		{
			float u = static_cast<float>(file.readFloat()) ;
			float v = static_cast<float>(file.readFloat()) ;
			surface.texCoords.push_back((Vector2){ u, v });
			Vector3 p = surface.vertices[i];
		    surface.vertex.push_back(p);
            surface.bounds.min = Vector3Min(surface.bounds.min, p);
            surface.bounds.max = Vector3Max(surface.bounds.max, p);

            bounds.min = Vector3Min(bounds.min, p);
            bounds.max = Vector3Max(bounds.max, p);

		}


	 


	 


	     surface.init();
        
         m_surfaces.push_back(surface);





		delete [] m_pSkins;    

 

 
		meshOffset += meshHeader.meshSize;
	}




    return true; 
}



void LoadMD3::update(int currentFrame, int nextFrame, float pol) 
{
    if (m_surfaces.empty()) return;

    int totalFrames = numFrames;  
    if (currentFrame >= totalFrames) currentFrame = totalFrames - 1;
    if (nextFrame >= totalFrames) nextFrame = totalFrames - 1;
    m_frame = currentFrame;
    m_nextFrame = nextFrame;
    m_poll = pol;
   UpdateTags(currentFrame, nextFrame, pol);

 
     

    
}



void LoadMD3::render(Shader shader, Matrix mat , Matrix parent)
{
 

    local = MatrixMultiply(mat,parent);

    Matrix matView = rlGetMatrixModelview();
    Matrix matProjection = rlGetMatrixProjection();
    Matrix matModelView = MatrixMultiply(local, matView);
    Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

	 int locIndex = GetShaderLocation(shader, "mvp");
    rlSetUniformMatrix(locIndex, matModelViewProjection);

 
    bounds.min = {FLT_MAX, FLT_MAX, FLT_MAX};
    bounds.max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};


   

 
 
    for (u32 i = 0; i < m_surfaces.size(); i++)
    {
     
		const tMD3Surface &surface = m_surfaces[i];
        BoundingBox _bounds;
 
 
        int numVertices = surface.vertex.size();

        if (surface.tex!=0)
		{
			rlActiveTextureSlot(0);
			rlEnableTexture(surface.tex);
			
		}

		if (m_surfaces[i].numFrames > 1)
		{
              _bounds.min = {FLT_MAX, FLT_MAX, FLT_MAX};
              _bounds.max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
			
					int currentOffsetVertex = m_frame * numVertices;
					int nextOffsetVertex = m_nextFrame * numVertices;
					for (int j = 0; j < numVertices; j++)
					{
						Vector3 pos =  Vector3Lerp(surface.vertices[currentOffsetVertex +j], surface.vertices[nextOffsetVertex + j], m_poll);

						m_surfaces[i].vertex[j].x =  pos.x; 
						m_surfaces[i].vertex[j].y =  pos.y;
						m_surfaces[i].vertex[j].z =  pos.z;



                    if (pos.x < _bounds.min.x) _bounds.min.x = pos.x;
                    if (pos.y < _bounds.min.y) _bounds.min.y = pos.y;
                    if (pos.z < _bounds.min.z) _bounds.min.z = pos.z;
                    if (pos.x > _bounds.max.x) _bounds.max.x = pos.x;
                    if (pos.y > _bounds.max.y) _bounds.max.y = pos.y;
                    if (pos.z > _bounds.max.z) _bounds.max.z = pos.z;


                    // if (pos.x < bounds.min.x) bounds.min.x = pos.x;
                    // if (pos.y < bounds.min.y) bounds.min.y = pos.y;
                    // if (pos.z < bounds.min.z) bounds.min.z = pos.z;
                    // if (pos.x > bounds.max.x) bounds.max.x = pos.x;
                    // if (pos.y > bounds.max.y) bounds.max.y = pos.y;
                    // if (pos.z > bounds.max.z) bounds.max.z = pos.z;
	

					 }
					m_surfaces[i].update();
			
		}else 
        {
            _bounds = surface.bounds;
        }
         m_surfaces[i].transformedBounds = TransformBoundingBox(_bounds, local);


         ExpandBoundingBox(bounds, _bounds);
         
         m_surfaces[i].render();
         
        }
        
//bounds=TransformBoundingBox(bounds, local);        
DrawBoundingBox(bounds, RED);
 

 for (u32 i = 0; i < numOfTags; i++)
     {
          if (!m_links[i].link) continue;
      //  rlPushMatrix();
     //   rlMultMatrixf(MatrixToFloat(m_links[i].trasform));
     //   DrawCube((Vector3){0, 0, 0}, 0.05f, 0.05f, 0.05f,RED);  // agora desenhado com rotação
     //   rlPopMatrix();

      //    DrawCube(m_links[i].position,0.1,0.1,0.1,RED);
        //  m_links[i].link->trasform = m_links[i].trasform;

          m_links[i].link->render(shader, m_links[i].trasform, local);
           //m_links[i].link->rotation = m_links[i].rotation;

  
          
     }
   

   

 

}

bool LoadMD3::SetTexture(u32 index, u32  texture) 
{
     if (index >= m_surfaces.size() )
     {
 
         return false;
     }
     m_surfaces[index].tex =  texture;
  
 
     return true;
}

Node3D *LoadMD3::getBone(const char *name)
{
    //  for (u32 i = 0; i < numOfTags; i++)
    //  {
    //       if (strcmp(m_bones[i]->name.c_str(), name) == 0)
    //       {
    //            return m_bones[i];
    //       }
    //  }
     return nullptr; 
}


void LoadMD3::setLink(const char *name, LoadMD3 *link) 
{
     for (u32 i = 0; i < numOfTags; i++)
     {
          if (strcmp(m_links[i].name, name) == 0)
          {
               m_links[i].link = link;
               return;
          }
     }
     LogWarning("Link %s nao encontrado", name);
}

Vector3 LoadMD3::GetTagPosition(u32 index) 
{
      return m_links[index].position;
}

bool LoadMD3::pick(Ray& ray)
{

        for (u32 i = 0; i < m_surfaces.size(); i++)
        {
            if (m_surfaces[i].pick(ray,local))
            {
                return true;
            }
        }
    // RayCollision result = GetRayCollisionBox(ray,bounds); 
    // if (result.hit) 
    // {   
    //     DrawBoundingBox(bounds, RED);
    //     for (u32 i = 0; i < m_surfaces.size(); i++)
    //     {
    //         if (m_surfaces[i].pick(ray,local))
    //         {
    //             return true;
    //         }
    //     }

    // }
    return false;
}


void LoadMD3::UpdateTags(int currentFrame, int nextFrame, float pol)
{
     int currentOffset = currentFrame * numOfTags;
     int nextOffset = nextFrame * numOfTags;


     for (u32 i = 0; i < numOfTags; i++)
     {
         Vector3 final_position = Vector3Lerp(m_tags[currentOffset + i].position, m_tags[nextOffset + i].position, pol);

    
        
         Mat3 rotA = Mat3FromMD3AxisColumn(m_tags[currentOffset + i].axes);
         Quaternion quatA = Mat3ToQuaternion(rotA);
         Mat3 rotB = Mat3FromMD3AxisColumn(m_tags[nextOffset + i].axes);
          Quaternion quatB = Mat3ToQuaternion(rotB);

         Quaternion final_rotation = QuaternionSlerp(quatA, quatB, pol);


         
        m_links[i].position = final_position;
         m_links[i].trasform= QuaternionToMatrix(final_rotation);


        m_links[i].trasform= MatrixIdentity();         
        m_links[i].trasform.m0 = m_tags[currentOffset + i].axes[0].x;
        m_links[i].trasform.m1 = m_tags[currentOffset + i].axes[0].y;
        m_links[i].trasform.m2 = m_tags[currentOffset + i].axes[0].z;
        m_links[i].trasform.m4 = m_tags[currentOffset + i].axes[1].x;
        m_links[i].trasform.m5 = m_tags[currentOffset + i].axes[1].y;
        m_links[i].trasform.m6 = m_tags[currentOffset + i].axes[1].z;
        m_links[i].trasform.m8 = m_tags[currentOffset + i].axes[2].x;
        m_links[i].trasform.m9 = m_tags[currentOffset + i].axes[2].y;
        m_links[i].trasform.m10 = m_tags[currentOffset + i].axes[2].z;
        
        
        m_links[i].trasform.m12 = final_position.x;
        m_links[i].trasform.m13 = final_position.y;
        m_links[i].trasform.m14 = final_position.z;
        
 
     }
}

void LoadMD3::Debug() 
{
    

    // for (u32 i = 0; i < m_surfaces.size(); i++)
    // {
       
    //     rlBegin(RL_LINES); 
    //     rlPushMatrix();
    //     for (u32 j = 0; j < m_surfaces[i].indices.size(); j++)
    //     {
    //         int i0 = m_surfaces[i].indices[j].vertexIndices [0];
    //         int i1 = m_surfaces[i].indices[j].vertexIndices [1];
    //         int i2 = m_surfaces[i].indices[j].vertexIndices [2];


    //         rlVertex3f(m_surfaces[i].vertices[i0].x, m_surfaces[i].vertices[i0].y, m_surfaces[i].vertices[i0].z);
    //         rlVertex3f(m_surfaces[i].vertices[i1].x, m_surfaces[i].vertices[i1].y, m_surfaces[i].vertices[i1].z);
          
    //         rlVertex3f(m_surfaces[i].vertices[i1].x, m_surfaces[i].vertices[i1].y, m_surfaces[i].vertices[i1].z);
    //         rlVertex3f(m_surfaces[i].vertices[i2].x, m_surfaces[i].vertices[i2].y, m_surfaces[i].vertices[i2].z);
          
    //         rlVertex3f(m_surfaces[i].vertices[i2].x, m_surfaces[i].vertices[i2].y, m_surfaces[i].vertices[i2].z);
    //         rlVertex3f(m_surfaces[i].vertices[i0].x, m_surfaces[i].vertices[i0].y, m_surfaces[i].vertices[i0].z);
           
    //     }
    //     rlPopMatrix();
    //     rlEnd();
    // }
    

}

void LoadMD3::update(float dtime) 
{
    m_animator->Update(dtime);
    int currentFrame, nextFrame;
    float interpolation;
    m_animator->GetCurrentFrames(currentFrame, nextFrame, interpolation);
    update(currentFrame, nextFrame, interpolation);
 
   
}

bool tMD3Surface::pick(Ray& ray, Matrix& mat)
{
    RayCollision result = GetRayCollisionBox(ray,transformedBounds); 
    if (result.hit) 
    {
        bool foundTriangle = false;
        float closestDistance = FLT_MAX;
        Vector3 closestP0, closestP1, closestP2;
        Vector3 closestPoint;
        
        for (u32 i = 0; i < indices.size(); i += 3)
        {
            Vector3 p0 = vertices[indices[i + 0]];
            Vector3 p1 = vertices[indices[i + 1]];
            Vector3 p2 = vertices[indices[i + 2]];
            
            p0 = Vector3Transform(p0, mat);
            p1 = Vector3Transform(p1, mat);
            p2 = Vector3Transform(p2, mat);
            
            RayCollision triangleResult = GetRayCollisionTriangle(ray, p0, p1, p2);
            
            if (triangleResult.hit)
            {
                // Verificar se este triângulo é mais próximo
                if (triangleResult.distance < closestDistance)
                {
                    closestDistance = triangleResult.distance;
                    closestP0 = p0;
                    closestP1 = p1;
                    closestP2 = p2;
                    closestPoint = triangleResult.point;
                    foundTriangle = true;
                }
            }
        }
        
        // Desenhar apenas o triângulo mais próximo
        if (foundTriangle)
        {
            DrawTriangle3D(closestP0, closestP1, closestP2, RED);
            
            // Opcional: desenhar um ponto no local da colisão
            DrawSphere(closestPoint, 0.05f, YELLOW);
            
            return true;
        }
    }
    return false;
}

 

void tMD3Surface::init()
{

	vertexCount  = vertex.size();

	LogInfo("Vertex count: %d", vertexCount);
	

    vaoId = rlLoadVertexArray();
    rlEnableVertexArray(vaoId);
    
    
    vboId[0] = rlLoadVertexBuffer(&vertex[0], vertexCount  * sizeof(Vector3), true);
    rlEnableVertexBuffer(vboId[0]); 
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    
    
    vboId[1] = rlLoadVertexBuffer(&texCoords[0], vertexCount  * sizeof(Vector2), false);
    rlEnableVertexBuffer(vboId[1]);
    rlEnableVertexAttribute(1);
    rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);

	vboId[2] = rlLoadVertexBufferElement(&indices[0], indices.size()   * sizeof(s16), false);
    rlEnableVertexBufferElement(vboId[2]);

 

	rlDisableVertexArray();

}
void tMD3Surface::update()
{
	if (vaoId == 0 || vertexCount == 0) return;
	rlUpdateVertexBuffer(vboId[0], vertex.data(), vertex.size()  * sizeof(Vector3), 0);
}


void tMD3Surface::render()
{
	if (vaoId == 0 || vertex.size() == 0 || indices.size() == 0) return;
	rlEnableVertexArray(vaoId);
	rlDrawVertexArrayElements(0, indices.size() , 0);
	rlDisableVertexArray();
}