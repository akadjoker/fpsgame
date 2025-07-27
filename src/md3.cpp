#include "md3.hpp"
#include <rlgl.h>
#include "raymath.h"
#define MAT(mat, row, col) (((float*)&mat)[col * 4 + row])



 
Quaternion quat(float x, float y, float z, float w) { return {x, y, z, w}; }

Quaternion Slerp(Quaternion &q1, Quaternion &q2, float t)
{
	// Create a local quaternion to store the interpolated quaternion
	Quaternion qInterpolated;


	if(q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w) 
		return q1;

	// Following the (b.a) part of the equation, we do a dot product between q1 and q2.
	// We can do a dot product because the same math applied for a 3D vector as a 4D vector.
	float result = (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.w * q2.w);

	// If the dot product is less than 0, the angle is greater than 90 degrees
	if(result < 0.0f)
	{
		// Negate the second quaternion and the result of the dot product
		q2 = quat(-q2.x, -q2.y, -q2.z, -q2.w);
		result = -result;
	}

	// Set the first and second scale for the interpolation
	float scale0 = 1 - t, scale1 = t;

	if(1 - result > 0.1f)
	{
		// Get the angle between the 2 quaternions, and then store the sin() of that angle
		float theta = (float)acos(result);
		float sinTheta = (float)sin(theta);

		// Calculate the scale for q1 and q2, according to the angle and it's sine value
		scale0 = (float)sin( ( 1 - t ) * theta) / sinTheta;
		scale1 = (float)sin( ( t * theta) ) / sinTheta;
	}	


	qInterpolated.x = (scale0 * q1.x) + (scale1 * q2.x);
	qInterpolated.y = (scale0 * q1.y) + (scale1 * q2.y);
	qInterpolated.z = (scale0 * q1.z) + (scale1 * q2.z);
	qInterpolated.w = (scale0 * q1.w) + (scale1 * q2.w);


	return qInterpolated;
}


Matrix CreateMatrix(Quaternion &q)
{
    Matrix pMatrix;
    // First row
    pMatrix.m0 = 1.0f - 2.0f * ( q.y * q.y + q.z * q.z );  
	pMatrix.m1 = 2.0f * ( q.x * q.y - q.w * q.z );  
	pMatrix.m2 = 2.0f * ( q.x * q.z + q.w * q.y );  
	pMatrix.m3 = 0.0f;  

	// Second row
	pMatrix.m4 = 2.0f * ( q.x * q.y + q.w * q.z );  
	pMatrix.m5 = 1.0f - 2.0f * ( q.x * q.x + q.z * q.z );  
	pMatrix.m6 = 2.0f * ( q.y * q.z - q.w * q.x );  
	pMatrix.m7 = 0.0f;  

	// Third row
	pMatrix.m8 = 2.0f * ( q.x * q.z - q.w * q.y );  
	pMatrix.m9 = 2.0f * ( q.y * q.z + q.w * q.x );  
	pMatrix.m10 = 1.0f - 2.0f * ( q.x * q.x + q.y * q.y );  
	pMatrix.m11 = 0.0f;  

	// Fourth row
	pMatrix.m12 = 0;  
	pMatrix.m13 = 0;  
	pMatrix.m14 = 0;  
	pMatrix.m15 = 1.0f;

    return pMatrix;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}

Quaternion CreateFromMatrix(float *pTheMatrix, int rowColumnCount)
{
    Quaternion q = {0, 0, 0, 1}; // Inicializar como identidade
    
    if(!pTheMatrix || ((rowColumnCount != 3) && (rowColumnCount != 4))) 
        return q;

    float *pMatrix = pTheMatrix;
    float m4x4[16] = {0};

    // Converter 3x3 para 4x4 se necessário
    if(rowColumnCount == 3)
    {
        // IMPORTANTE: Transpor a matriz 3x3 de row-major (MD3) para column-major (OpenGL)
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

    // Calcular o trace (diagonal)
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
    
    // Normalizar o quaternion
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

    // Interpolação da rotação (SLERP)
    Quaternion qInterpolated = QuaternionSlerp(qA, qB, t);

    // Criar matriz a partir do quaternion (já em column-major)
    Matrix result = QuaternionToMatrix(qInterpolated);

    // Colocar a posição nos campos corretos para column-major
    result.m12 = position.x;  // m[0][3] - posição X
    result.m13 = position.y;  // m[1][3] - posição Y  
    result.m14 = position.z;  // m[2][3] - posição Z

    return result;
}
    // Transpor ao copiar: MD3 é row-major, Mat3 é column-major
static void fromMD3Rotation(const float rotation[3][3], float result[3][3])
{
    // Transpor a matriz: result[col][row] = rotation[row][col]
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            result[col][row] = rotation[row][col];
        }
    }
}


Quaternion QuaternionFromRotationMatrix(const float matrix[9]) 
{
    Quaternion q;

   

    // Assumindo que matrix[9] está em formato row-major:
    // [0] [1] [2]
    // [3] [4] [5]  
    // [6] [7] [8]
    
    float m00 = matrix[0];  // [0,0]
    float m01 = matrix[1];  // [0,1]
    float m02 = matrix[2];  // [0,2]
    float m10 = matrix[3];  // [1,0]
    float m11 = matrix[4];  // [1,1]
    float m12 = matrix[5];  // [1,2]
    float m20 = matrix[6];  // [2,0]
    float m21 = matrix[7];  // [2,1]
    float m22 = matrix[8];  // [2,2]
    
    float t = m00 + m11 + m22;
    
    if (t > 0.0f)
    {
        float invS = 0.5f / sqrtf(1.0f + t);
        
        q.x = (m21 - m12) * invS;
        q.y = (m02 - m20) * invS;
        q.z = (m10 - m01) * invS;
        q.w = 0.25f / invS;
    }
    else
    {
        if (m00 > m11 && m00 > m22)
        {
            float invS = 0.5f / sqrtf(1.0f + m00 - m11 - m22);
            
            q.x = 0.25f / invS;
            q.y = (m01 + m10) * invS;
            q.z = (m20 + m02) * invS;
            q.w = (m21 - m12) * invS;
        }
        else if (m11 > m22)
        {
            float invS = 0.5f / sqrtf(1.0f + m11 - m00 - m22);
            
            q.x = (m01 + m10) * invS;
            q.y = 0.25f / invS;
            q.z = (m12 + m21) * invS;
            q.w = (m02 - m20) * invS;
        }
        else
        {
            float invS = 0.5f / sqrtf(1.0f + m22 - m00 - m11);
            
            q.x = (m02 + m20) * invS;
            q.y = (m12 + m21) * invS;
            q.z = 0.25f / invS;
            q.w = (m10 - m01) * invS;
        }
    }
    
    return q;
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





void DrawTria3D(Vector3 v1, Vector3 v2, Vector3 v3,Vector2 uv1,Vector2 uv2,Vector2 uv3, Color color)
{
    //rlCheckRenderBatchLimit(4);

   // rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(uv1.x, uv1.y);
        rlVertex3f(v1.x, v1.y, v1.z);

        rlTexCoord2f(uv2.x,uv2.y);
        rlVertex3f(v2.x, v2.y, v2.z);

        rlTexCoord2f(uv3.x,uv3.y);
        rlVertex3f(v3.x, v3.y, v3.z);


        rlTexCoord2f(uv1.x,uv1.y);
        rlVertex3f(v1.x, v1.y, v1.z);

  //  rlEnd();
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


		file.seek(meshOffset + meshHeader.uvStart, SEEK_SET);
		for (int i = 0; i < meshHeader.numVertices; i++)
		{
			float u = static_cast<float>(file.readFloat()) ;
			float v = static_cast<float>(file.readFloat()) ;
			surface.texCoords.push_back((Vector2){ u, v });
			Vector3 p = surface.vertices[i];
		    surface.vertex.push_back(p);

		}


	 


	 


	     surface.init();
        
         m_surfaces.push_back(surface);





		delete [] m_pSkins;    

 

 
		meshOffset += meshHeader.meshSize;
	}




    return false; 
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
 

    Matrix local = MatrixMultiply(mat,parent);

    Matrix matView = rlGetMatrixModelview();
    Matrix matProjection = rlGetMatrixProjection();
    Matrix matModelView = MatrixMultiply(local, matView);
    Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

	 int locIndex = GetShaderLocation(shader, "mvp");
    rlSetUniformMatrix(locIndex, matModelViewProjection);

 


   

 
 
    for (u32 i = 0; i < m_surfaces.size(); i++)
    {
     
		const tMD3Surface &surface = m_surfaces[i];
		int numVertices = surface.vertex.size();
		
		if (surface.tex!=0)
		{
			rlActiveTextureSlot(0);
			rlEnableTexture(surface.tex);
			
		}


        
	
		if (m_surfaces[i].numFrames > 1)
		{
			
					int currentOffsetVertex = m_frame * numVertices;
					int nextOffsetVertex = m_nextFrame * numVertices;
					for (int j = 0; j < numVertices; j++)
					{
						Vector3 pos =  Vector3Lerp(surface.vertices[currentOffsetVertex +j], surface.vertices[nextOffsetVertex + j], m_poll);

						m_surfaces[i].vertex[j].x =  pos.x; 
						m_surfaces[i].vertex[j].y =  pos.y;
						m_surfaces[i].vertex[j].z =  pos.z;
					 }
					m_surfaces[i].update();
			
		}

   m_surfaces[i].render();
  
}

 

 for (u32 i = 0; i < numOfTags; i++)
     {
          if (!m_links[i].link) continue;
      //  rlPushMatrix();
     //   rlMultMatrixf(MatrixToFloat(m_links[i].trasform));
     //   DrawCube((Vector3){0, 0, 0}, 0.05f, 0.05f, 0.05f,RED);  // agora desenhado com rotação
     //   rlPopMatrix();

      //    DrawCube(m_links[i].position,0.1,0.1,0.1,RED);

          m_links[i].link->render(shader, m_links[i].trasform,local);
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
          }
     }
}

Vector3 LoadMD3::GetTagPosition(u32 index) 
{
      return m_links[index].position;
}


void LoadMD3::UpdateTags(int currentFrame, int nextFrame, float pol)
{
     int currentOffset = currentFrame * numOfTags;
     int nextOffset = nextFrame * numOfTags;


     for (u32 i = 0; i < numOfTags; i++)
     {
         Vector3 position = m_tags[currentOffset + i].position;
         //Vector3Lerp(m_tags[currentOffset + i].position,
           //          m_tags[nextOffset + i].position, pol);

         //  Quaternion rotCurrent = CreateFromMatrix(m_tags[currentOffset +
         //  i].rotation,3); Quaternion rotNext   =
         //  CreateFromMatrix(m_tags[nextOffset + i].rotation,3);
         // Quaternion  rotation = QuaternionSlerp(rotCurrent, rotNext, pol);


         //Matrix mat =  QuaternionToMatrix(rotation);

       // m_links[i].trasform = BuildTagMatrix(m_tags[currentOffset + i], m_tags[nextOffset + i], pol);
        //  MatrixTranspose(QuaternionToMatrix(rotation));
        
        Mat3 rotA = Mat3FromMD3AxisColumn(m_tags[currentOffset + i].axes);
        Quaternion quatA = Mat3ToQuaternion(rotA);

        Mat3 rotB = Mat3FromMD3AxisColumn(m_tags[nextOffset + i].axes);
        Quaternion quatB = Mat3ToQuaternion(rotB);

        Quaternion rotation = QuaternionSlerp(quatA, quatB, pol);


        m_links[i].trasform= MatrixIdentity();
        m_links[i].position = position;

        //Matrix traslation = MatrixTranslate(position.x, position.y, position.z);

       // m_links[i].trasform= MatrixMultiply(QuaternionToMatrix(rotation),traslation);
        
        m_links[i].trasform.m0 = m_tags[currentOffset + i].axes[0].x;
        m_links[i].trasform.m1 = m_tags[currentOffset + i].axes[0].y;
        m_links[i].trasform.m2 = m_tags[currentOffset + i].axes[0].z;

        m_links[i].trasform.m4 = m_tags[currentOffset + i].axes[1].x;
        m_links[i].trasform.m5 = m_tags[currentOffset + i].axes[1].y;
        m_links[i].trasform.m6 = m_tags[currentOffset + i].axes[1].z;

        m_links[i].trasform.m8 = m_tags[currentOffset + i].axes[2].x;
        m_links[i].trasform.m9 = m_tags[currentOffset + i].axes[2].y;
        m_links[i].trasform.m10 = m_tags[currentOffset + i].axes[2].z;
        
        
        
        m_links[i].trasform.m12 = position.x;
        m_links[i].trasform.m13 = position.y;
        m_links[i].trasform.m14 = position.z;

        // if (m_links[i].link)
        // {
        //     m_links[i].link->trasform = m_links[i].trasform;
        // }


            // m[0] = MAT(matrix,0,0); m[4] = MAT(matrix,0,1); m[8] = MAT(matrix,0,2); m[12] = position[0];
			// m[1] = MAT(matrix,1,0); m[5] = MAT(matrix,1,1); m[9] = MAT(matrix,1,2); m[13] = position[1];
			// m[2] = MAT(matrix,2,0); m[6] = MAT(matrix,2,1); m[10]= MAT(matrix,2,2); m[14] = position[2];
			// m[3] = 0;               m[7] = 0;               m[11]= 0;               m[15] = 1;


        // m_links[i].trasform = mat;
//         BuildMatrixFromRotationAndPosition(mat, m_links[i].position);
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