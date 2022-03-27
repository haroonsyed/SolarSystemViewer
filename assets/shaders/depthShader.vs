#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 transformedPos;

uniform mat4 model;
uniform mat4 view;

void main()
{
   transformedPos = vec3(model * vec4(aPos,1));

   // Calculate Position
   gl_Position = view * vec4(transformedPos,1);
   
};