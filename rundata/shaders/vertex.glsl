#version 430 core
layout (location=0) in vec3 Pos;

layout (location=0) uniform mat4 Transform;
void main()
{
    gl_Position = Transform * vec4(Pos, 1.f);
}
