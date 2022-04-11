#version 330 core
in vec3 transformedPos;
in vec3 transformedNorm;
in vec2 uvCoord;
in mat3 TBN;

out vec4 FragColor;

uniform vec3 lightPos;

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
  vec3 toLight = normalize(lightPos-transformedPos);
  vec3 viewDir = normalize(transformedPos);
  vec3 h = normalize((-1*viewDir) + toLight);

  float ambient = 0.1;
  float specularStrength = 0.5f;
  float phongExp = 100.0f;

  float diffuse = max(dot(normal,toLight),0);
  float specular = pow(max(dot(normal,h), 0),phongExp);

  vec4 specularColor = specularMapExists ? specularMapData : vec4(1.0);
  
  FragColor = (ambient + diffuse) * diffuseColor + specularStrength * specular * specularColor;
};