#version 430 core
out vec4 FragColor;

in vec2 uvCoord;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomTexture0;
layout(binding = 2) uniform sampler2D bloomTexture1;
layout(binding = 3) uniform sampler2D bloomTexture2;
layout(binding = 4) uniform sampler2D bloomTexture3;
layout(binding = 5) uniform sampler2D bloomTexture4;

void main()
{

  float bloomStrength = 0.04;

  vec4 hdrColor = texture(screenTexture, uvCoord);
  vec4 blurColor = texture(bloomTexture0, uvCoord);
  vec4 pixelColor = mix(hdrColor, blurColor, bloomStrength);

  // Perform hdr tone-mapping
  const float gamma = 2.2;

  // tone mapping
  const float exposure = 1e-3;
  vec3 mapped = vec3(1.0) - exp(-pixelColor.rgb * exposure);

  // Gamma correction
  mapped = pow(mapped, vec3(1.0 / gamma));
  
  FragColor = vec4(mapped, pixelColor.a);

};