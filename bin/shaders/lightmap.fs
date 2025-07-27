#version 330

in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;

uniform sampler2D texture0;      // Difusa
uniform sampler2D texture1;      // Lightmap
uniform float lightmapBlend;     

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord) ; 
    //if (texColor.a < 0.1) discard;


    vec4 lightmapColor = texture(texture1, fragTexCoord2);


   // vec4 modulated =   lightmapColor * texColor ;
   // vec4 blended = mix(modulated, texColor, lightmapBlend);

 
   // finalColor = blended * fragColor;

       vec3 result = texColor.rgb + lightmapColor.rgb ;

    // Modula com a cor do vértice e o fator de lightmapBlend
    result *= mix(vec3(0.5), fragColor.rgb, lightmapBlend);

    finalColor = vec4(result, texColor.a);


   // vec3 result =   (lightmapColor + texColor) ;

    //finalColor = vec4(result, 1.0);// * (fragColor * lightmapBlend) ;
     
}
