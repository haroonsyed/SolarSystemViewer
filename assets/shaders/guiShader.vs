#version 330 core
layout (location = 0) in vec3 aPos;

uniform float scale;

out vec3 position;

void main()
{
   gl_Position = vec4(aPos, 1.0);
   position = aPos;
};