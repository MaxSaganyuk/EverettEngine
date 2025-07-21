#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
flat in ivec4 BoneIDs;
in vec4 Weights;
 
uniform vec3 viewPos;

uniform vec3 ambient;

uniform Material material;
uniform DirLight dirLight;

#define LIGHT_MAX_AMOUNT 10

uniform int dirLightAmount; 
uniform DirLight dirLights[LIGHT_MAX_AMOUNT];

uniform int pointLightAmount;
uniform PointLight pointLights[LIGHT_MAX_AMOUNT];

uniform int spotLightAmount;
uniform SpotLight spotLights[LIGHT_MAX_AMOUNT];

uniform int boneIDChoice;

vec3 AmbientLight(vec3 normal)
{
    vec3 amb = (ambient * vec3(texture(material.diffuse, TexCoords)));

    return amb;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    bool found = false;
    vec3 res = AmbientLight(norm);
     for (int i = 0 ; i < 4 ; i++) 
     {
        if (BoneIDs[i] == boneIDChoice) 
        {
           if (Weights[i] >= 0.7) 
           {
               FragColor = vec4(1.0, 0.0, 0.0, 0.0) * Weights[i];
           } 
           else if (Weights[i] >= 0.4 && Weights[i] <= 0.6)
           {
               FragColor = vec4(0.0, 1.0, 0.0, 0.0) * Weights[i];
           } 
           else if (Weights[i] >= 0.1)
           {
               FragColor = vec4(1.0, 1.0, 0.0, 0.0) * Weights[i];
           }

           found = true;
        }
    }
    if(!found)
    {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
}