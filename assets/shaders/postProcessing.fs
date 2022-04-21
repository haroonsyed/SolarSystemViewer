#version 330 core
out vec4 FragColor;

in vec2 uvCoord;

uniform sampler2D screenTexture;

void main()
{
   FragColor = texture(screenTexture, uvCoord);
};