#version 430 core
out vec4 FragColor;

in vec2 uvCoord;

uniform sampler2D screenTexture;
uniform float exposure;

void main()
{
  vec4 hdrColorRaw = texture(screenTexture, uvCoord);
  vec3 hdrColor = hdrColorRaw.rgb;

  // Perform luminance mapping

  // Perform hdr tone-mapping
  const float gamma = 2.2;

  // exposure tone mapping
  vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
  //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

  // Gamma correction
  mapped = pow(mapped, vec3(1.0 / gamma));
  
  FragColor = vec4(mapped, hdrColorRaw.w);
  //FragColor = texture(screenTexture, uvCoord);

};