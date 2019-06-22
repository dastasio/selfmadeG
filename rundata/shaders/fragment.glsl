#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
in vec3 FragmentPosition;
in vec3 FragmentNormal;
//in vec2 FragmentTextureCoordinates;

layout (location=1) uniform vec3 CameraPosition;
layout (location=3) uniform vec3 LightData[2];
//layout (location=5) uniform sampler2D Texture;

out vec4 FragmentColor;
vec3 ComputeDirectionalLight(vec3 LightColor, vec3 LightDirection, vec3 FragmentPosition)
{
    vec3 Normal = normalize(FragmentNormal);
    float AmbientIntensity = 0.3f;
    vec3 Ambient = AmbientIntensity*LightColor;

    float DiffuseIntensity = 0.8*max(dot(Normal, -normalize(LightDirection)), 0f);
    vec3 Diffuse = DiffuseIntensity*LightColor;

    return(Ambient + Diffuse);
}

vec3 ComputePointLight(vec3 LightColor, vec3 LightPosition, vec3 FragmentPosition)
{
    vec3 Normal = normalize(FragmentNormal);
    float AmbientIntensity = 0.1f;
    vec3 Ambient = AmbientIntensity*LightColor;

    vec3 LightDirection = normalize(LightPosition - FragmentPosition);
    float DiffuseIntensity = max(dot(Normal, LightDirection), 0f);
    vec3 Diffuse = DiffuseIntensity*LightColor;

    vec3 ReflectedLight = reflect(-LightDirection, Normal);
    vec3 CameraDirection = normalize(CameraPosition - FragmentPosition);
    float SpecularIntensity = 0.5*pow(max(dot(CameraDirection, ReflectedLight), 0f), 16);
    vec3 Specular = SpecularIntensity*LightColor;

    return(Ambient + Diffuse + 0*Specular);
}

void main()
{
    vec3 LightColor = LightData[1];
    vec3 LightPosition = LightData[0];
    vec3 MaterialColor = vec3(0.9f, 0.9f, 0.8f);

    vec3 Light = ComputePointLight(LightColor, LightPosition, FragmentPosition);
    FragmentColor = vec4(Light*MaterialColor, 1.f);
}
