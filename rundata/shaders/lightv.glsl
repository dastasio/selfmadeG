#version 430 core
layout (location=0) in vec3 VertexPosition;

layout (location=0) uniform mat4 CameraSpaceTransform;
layout (location=2) uniform mat4 ModelTransform;
void main()
{
    gl_Position = CameraSpaceTransform * ModelTransform * vec4(VertexPosition, 1f);
}
