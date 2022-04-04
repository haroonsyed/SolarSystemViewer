#version 330 core
in vec3 transformedPos;
in vec2 uvCoord;
in vec3 transformedNorm;

out vec4 FragColor;

uniform vec3 lightPos;
uniform mat4 view;

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
  vec3 viewDir = normalize(transformedPos - vec3(view));
  vec3 toLight = normalize(lightPos-transformedPos);
  vec3 h = normalize((-1*viewDir) + toLight);

  float ambient = 0.1;
  float specularStrength = 1;
  float phongExp = 100;

  float diffuse = max(dot(normal,toLight),0);
  float specular = max(pow(dot(normal,h), phongExp),0);

  vec4 specularColor = vec4(1.0);
  
  FragColor = (ambient + diffuse) * diffuseColor + specularStrength * specular * specularColor;
};