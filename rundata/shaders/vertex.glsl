#version 400 core
layout (location=0) in vec2 Pos;

void main()
{
    gl_Position = vec4(Pos, 0.f, 1.f);
}
