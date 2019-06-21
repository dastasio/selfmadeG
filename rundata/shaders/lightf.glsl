#version 430 core

layout (location = 3) uniform vec3 LightColor;
out vec4 FragmentColor;
void main()
{
    FragmentColor = vec4(LightColor, 1f);
}
