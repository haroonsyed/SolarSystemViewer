#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

out vec3 transformedPos;
out vec3 transformedNorm;
out vec2 uvCoord;

uniform mat4 modelView;
uniform mat4 projection;

void main()
{
   transformedPos = vec3(modelView * vec4(aPos,1));
   transformedNorm = vec3(modelView * vec4(aNorm,0.0));
   uvCoord = aTex;

   // Calculate Position
   gl_Position = projection * modelView * vec4(aPos,1);

};