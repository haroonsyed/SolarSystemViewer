#version 330 core
in vec3 transformedPos;
in vec2 uvCoord;
in vec3 transformedNorm;

out vec4 FragColor;

uniform vec3 lightPos;
uniform sampler2D imageTex;

void main()
{
	
  // Grab color for fragment from texture
  vec4 imgTexColor = texture(imageTex, uvCoord);

  // Calculate vectors needed for lighting
  vec3 normal = normalize(transformedNorm);
  vec3 viewDir = normalize(transformedPos);
  vec3 toLight = normalize(lightPos-transformedPos);
  vec3 h = normalize((-1*viewDir) + toLight);

  float ambient = 0.1;
  float specularStrength = 1;
  float phongExp = 100;

  float diffuse = max(dot(normal,toLight),0);
  float specular = max(pow(dot(normal,h), phongExp),0);
  
  FragColor = (ambient + diffuse) * imgTexColor + specularStrength * specular * vec4(1.0);
};