#version 430 core
layout (location = 0) in vec4 aPos;

layout (std140, binding = 0) uniform uniformData
{
    mat4 view;
    mat4 projection;
};

bool aboutEqualsVec2(vec2 v1, vec2 v2) {
  vec2 diff = abs(v2-v1);
  const float epsilon = 1e-5;
  return ( diff.x < epsilon && diff.y < epsilon );
}

void main()
{

   gl_Position = projection * view * vec4(aPos.xyz, 1.0);

};