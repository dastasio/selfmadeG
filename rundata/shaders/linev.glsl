#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

layout(location=0) in float Vertex;

layout(location=1) uniform mat4 CameraProjection;
layout(location=2) uniform vec3 Start;
layout(location=3) uniform vec3 End;

void main()
{
    if(Vertex == 0.f)
    {
        gl_Position = CameraProjection * vec4(Start, 1.f);
    }
    else
    {
        gl_Position = CameraProjection * vec4(End, 1.f);
    }
}
