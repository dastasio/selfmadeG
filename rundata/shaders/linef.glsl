#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

layout(location=4) uniform vec3 LineColor;

out vec4 Color;
void main()
{
    Color = vec4(LineColor, 1.f);
}
