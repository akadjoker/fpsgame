#version 330

 
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec2 vertexTexCoord2;
in vec4 vertexColor;
 

 
uniform mat4 mvp;
  


 
out vec2 fragTexCoord;
out vec2 fragTexCoord2;
out vec4 fragColor;
 

void main()
{
  
   
    fragTexCoord = vertexTexCoord;
    fragTexCoord2 = vertexTexCoord2;
    fragColor = vertexColor;

    
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
