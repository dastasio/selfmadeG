#version 430 core
layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec3 VertexNormal;

layout (location=0) uniform mat4 Transform;

out vec3 FragmentPosition;
out vec3 FragmentNormal;
void main()
{
    FragmentNormal = normalize(VertexNormal);
    FragmentPosition = VertexPosition;
    gl_Position = Transform * vec4(VertexPosition, 1f);;
}
