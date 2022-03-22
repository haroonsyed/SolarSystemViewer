#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aColor;

out vec4 vertexColor;
out vec3 transformedPos;
out vec3 transformedNorm;

uniform mat4 model;
uniform mat4 view;
uniform vec3 lightPos;

void main()
{
   transformedPos = vec3(model * vec4(aPos,1));

   // Calculate Position
   gl_Position = view * vec4(transformedPos,1);
   
   vertexColor = vec4(aColor, 1.0);
};