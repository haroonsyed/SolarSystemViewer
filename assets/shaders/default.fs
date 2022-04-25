#version 410 core
in vec3 transformedPos;
in vec3 transformedNorm;
in vec2 uvCoord;
in mat3 TBN;

out vec4 FragColor;

// x,y,z,type(point/spotlight),r,g,b,strength
const int numLightAttr = 8;
const int maxNumLights = 20;
uniform float lights[numLightAttr*maxNumLights]; // Each light has 8 attributes, max of 20 lights
uniform int lightCount; // How many lights to render for this frame

uniform bool diffuseMapExists;
uniform bool normalMapExists;
uniform bool specularMapExists;
uniform bool emissiveMapExists;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D emissiveMap;

void main()
{
	
  // Grab color for fragment from texture
  vec4 diffuseColor = texture(diffuseMap, uvCoord);
  vec3 normalMapData = texture(normalMap, uvCoord).rgb;
  vec4 specularMapData = texture(specularMap, uvCoord);
  vec4 emissiveMapData = texture(emissiveMap, uvCoord);

  // Calculate vectors needed for lighting
  vec3 normal = normalize(transformedNorm);
  if (normalMapExists) {
		normal = normalMapData * 2.0 - 1.0; // Make range between -1 and 1 for normal map
		normal = normalize(TBN * normal);
  }
	
  // Iterate through the lights and add each result to the FragColor
  FragColor = vec4(0.0, 0.0, 0.0, 1.0);
  for (int i=0; i<lightCount; i++) {
	
		int lightIndex = numLightAttr * i;
	
		// x,y,z,type(point/spotlight),r,g,b,strength
		vec3 lightPos = vec3(lights[lightIndex], lights[lightIndex+1], lights[lightIndex+2]);
		vec4 lightColor = vec4(lights[lightIndex+4], lights[lightIndex+5], lights[lightIndex+6], 1.0);
		float lightStrength = lights[lightIndex+7];
	
		vec3 toLight = normalize(lightPos-transformedPos);
		vec3 viewDir = normalize(transformedPos);
		vec3 h = normalize((-1*viewDir) + toLight);
	
		float ambient = 0.1;
		float specularStrength = 0.01f;
		float phongExp = 1.0f;
	
		float diffuse = max(dot(normal,toLight),0);
		float specular = pow(max(dot(normal,h), 0),phongExp);
	
		vec4 specularColor = specularMapExists ? specularMapData : vec4(1.0);
	
		float distance = length(lightPos - transformedPos);
		float Kc = 1.0;
		float Kl = 0.0014 / 2;
		float Kq = 0.000007 / 4;
		float attenuation = 1.0/( Kc + Kl * distance + Kq * distance * distance );
		lightStrength *= attenuation;
		
		FragColor += lightStrength * ( (ambient + diffuse) * diffuseColor * lightColor + specularStrength * specular * specularColor );
  }

};