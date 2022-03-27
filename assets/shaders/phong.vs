#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

out vec4 vertexColor;
out vec3 transformedPos;
out vec3 transformedNorm;

uniform mat4 model;
uniform mat4 view;

void main()
{
   transformedPos = vec3(model * vec4(aPos,1));
   transformedNorm = vec3(model * vec4(aNorm,0.0));

   // Calculate Position
   gl_Position = view * vec4(transformedPos,1);
   
   vertexColor = vec4(1.0, 0.0, 0.0, 1.0);
};