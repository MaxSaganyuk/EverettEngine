#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
 
uniform vec3 viewPos;

uniform Material material;
uniform Light light;


void main()
{
    vec3 lightDir = normalize(light.position - FragPos);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    diffuse *= intensity;
    specular *= intensity;

    float distance = length(light.position - FragPos);
    float atten = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    ambient *= atten;
    diffuse *= atten;
    specular *= atten;

    vec3 res = ambient + diffuse + specular;
    FragColor = vec4(res, 1.0);
}