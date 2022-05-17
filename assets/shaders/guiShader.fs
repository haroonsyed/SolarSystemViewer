#version 430 core
out vec4 FragColor;
in vec3 position;

uniform float leftBound;
uniform float rightBound;
uniform float topBound;
uniform float bottomBound;

void main()
{
   if (position.x < leftBound || position.x > rightBound || position.y > topBound || position.y < bottomBound)
	{
	   FragColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	}
};