#version 330 core
out vec4 FragColor;  

in vec2 texCoord;

uniform sampler2D ourTexture;
uniform sampler2D secondTexture;
  
void main()
{
    FragColor = mix(texture(ourTexture, texCoord), texture(secondTexture, texCoord), 0.4);
}