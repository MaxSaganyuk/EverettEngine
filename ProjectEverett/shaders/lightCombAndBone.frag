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

vec3 AmbientLight(vec3 normal)
{
    vec3 amb = (ambient * vec3(texture(material.diffuse, TexCoords)));

    return amb;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    return (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
 
    float diff = max(dot(normal, lightDir), 0.0);
 
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
 
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
 
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (diffuse + specular);    
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - FragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float distance = length(light.position - FragPos);
    float atten = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    diffuse *= intensity;
    specular *= intensity;

    diffuse *= atten;
    specular *= atten;

    return (diffuse + specular);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 res = AmbientLight(norm);

    for(int i = 0; i < dirLightAmount; ++i)
    {
        res += CalcDirLight(dirLights[i], norm, viewDir);
    }

    for(int i = 0; i < pointLightAmount; ++i)
    {
        res += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    for(int i = 0; i < spotLightAmount; ++i)
    {
        res += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }

    // For textureless model testing
    // FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    
    FragColor = vec4(res, 1.0);
}