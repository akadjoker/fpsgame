#version 330

in vec2 fragTexCoord;



uniform sampler2D texture0;      



out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord) ; 
   
    finalColor =  texColor ;


     
}
