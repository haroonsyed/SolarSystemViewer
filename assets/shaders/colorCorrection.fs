#version 430 core
out vec4 FragColor;

in vec2 uvCoord;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomTexture;
uniform float exposure;

void main()
{
  vec4 hdrColorRaw = texture(screenTexture, uvCoord) + texture(bloomTexture, uvCoord);
  //vec4 hdrColorRaw = texture(bloomTexture, uvCoord);
  vec3 hdrColor = hdrColorRaw.rgb;

  // Add bloom texture if applicable
  //vec4 blurColor = texture(bloomTexture, uvCoord);
  //imageStore(hdrImage, pixelPosition, ivec4(pixelRaw + blurColor) );
  //imageStore(hdrImage, pixelPosition, ivec4(blurColor) );

  // Perform hdr tone-mapping
  const float gamma = 2.2;

  // exposure tone mapping
  vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
  //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

  // Gamma correction
  //mapped = pow(mapped, vec3(1.0 / gamma));
  
  FragColor = vec4(mapped, hdrColorRaw.w);
  //FragColor = texture(screenTexture, uvCoord);

};