#version 430 core
uniform samplerCube skybox;
out vec4 FragColor;
in vec3 TexCoords;

void main()
{
   FragColor = texture(skybox, TexCoords);
};