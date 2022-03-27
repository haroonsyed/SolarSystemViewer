#version 330 core
out vec4 FragColor;

void main()
{
   // 1.5 makes the depth shading a bit stronger. 
   // According to learnopengl.com, the z values are on a log scale. Pow2 reverses this
   // Points closer are darker
   FragColor = vec4(vec3( 1.5*pow(gl_FragCoord.z,2) ), 1.0);
};