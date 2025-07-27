#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;       
  

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord)  ; 
  //  if (texColor.a < 0.1 || fragColor.a < 0.1) discard;
     finalColor = texColor * fragColor;
     
}
