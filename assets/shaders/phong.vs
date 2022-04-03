#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

out vec3 transformedPos;
out vec2 uvCoord;
out vec3 transformedNorm;

uniform mat4 scale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   transformedPos = vec3(model * scale * vec4(aPos,1));
   transformedNorm = vec3(model * scale * vec4(aNorm,0.0));
   uvCoord = aTex;

   // Calculate Position
   gl_Position = projection * view * model * scale * vec4(aPos,1);

};