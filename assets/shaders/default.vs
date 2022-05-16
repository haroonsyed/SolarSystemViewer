#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNorm;
layout (location = 3) in vec3 aTan;
layout (location = 4) in mat4 model;

out vec3 transformedPos;
out vec3 transformedNorm;
out vec2 uvCoord;
out mat3 TBN;
out vec4 texLoc;

const int numLightAttr = 8;
const int maxNumLights = 20;
layout (std140, binding = 0) uniform uniformData
{
    mat4 view;
    mat4 projection;
		int lightCount; // How many lights to render for this frame
		vec4 lights[numLightAttr*maxNumLights / 4];  // Each light has 8 attributes, max of 20 lights / 4 since each is a vec4 for tight packing
		float ambientStrength;
		float specularStrength;
		float phongExponent;
		float kc;
		float kl;
		float kq;
};

void main()
{
   mat4 modelView = view * model;
   transformedPos = vec3(modelView * vec4(aPos,1));
   transformedNorm = vec3(modelView * vec4(aNorm,0.0));
   uvCoord = aUV;

   // Create TBN matrix for normal calculation
   vec3 T = normalize(vec3(modelView * vec4(aTan, 0.0)));
   vec3 N = normalize(vec3(transformedNorm));
   vec3 B = cross(N, T);
   TBN = mat3(T, B, N);

   // Calculate Position
   gl_Position = projection * modelView * vec4(aPos,1);

};