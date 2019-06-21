#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec3 VertexNormal;

layout (location=0) uniform mat4 CameraSpaceTransform;
layout (location=2) uniform mat4 ModelTransform;

out vec3 FragmentPosition;
out vec3 FragmentNormal;
void main()
{
    FragmentNormal = mat3(ModelTransform) * VertexNormal;
    FragmentPosition = (ModelTransform * vec4(VertexPosition, 1f)).xyz;
    gl_Position = CameraSpaceTransform * ModelTransform * vec4(VertexPosition, 1f);
}
