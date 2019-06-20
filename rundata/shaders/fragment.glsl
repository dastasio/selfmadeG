#version 430 core
in vec3 FragmentPosition;
in vec3 FragmentNormal;

layout (location=1) uniform vec3 CameraPosition;

out vec4 FragmentColor;
vec3 ComputeLight(vec3 LightColor, vec3 FragmentPosition)
{
    vec3 LightPosition = vec3(0.f, 3.f, 0.f);
    float AmbientIntensity = 0.1f;

    vec3 LightDirection = normalize(LightPosition - FragmentPosition);
    float DiffuseIntensity = max(dot(FragmentNormal, LightDirection), 0f);

    vec3 ReflectedLight = reflect(-LightDirection, FragmentNormal);
    vec3 CameraDirection = normalize(CameraPosition - FragmentPosition);
    float SpecularIntensity = 0.5*pow(max(dot(ReflectedLight, CameraDirection), 0f), 16);

    return((AmbientIntensity + DiffuseIntensity + SpecularIntensity)*LightColor);
}

void main()
{
    vec3 LightColor = vec3(1f);
    vec3 MaterialColor = vec3(1.f, 0.7f, 0.6f);

    vec3 Light = ComputeLight(LightColor, FragmentPosition);
    FragmentColor = vec4(Light*MaterialColor, 1f);
}
