#version 430 core
out vec4 FragColor;

in vec2 uvCoord;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomTexture1;
layout(binding = 2) uniform sampler2D bloomTexture2;
layout(binding = 3) uniform sampler2D bloomTexture3;
layout(binding = 4) uniform sampler2D bloomTexture4;
layout(binding = 5) uniform sampler2D lightPass;

uniform float exposure;

void main()
{
  vec4 bloomColor = texture(bloomTexture1, uvCoord) + texture(bloomTexture2, uvCoord) + texture(bloomTexture3, uvCoord) + texture(bloomTexture4, uvCoord);
  bloomColor /= 4;
  vec4 pixelColor = texture(screenTexture, uvCoord) + bloomColor;

  // Perform hdr tone-mapping
  const float gamma = 2.2;

  // tone mapping
  vec3 mapped = vec3(1.0) - exp(-pixelColor.rgb * exposure);
  //vec3 mapped = pixelColor.rgb / (pixelColor.rgb + vec3(1.0));

  // Gamma correction
  mapped = pow(mapped, vec3(1.0 / gamma));
  
  FragColor = vec4(mapped, pixelColor.a);

};