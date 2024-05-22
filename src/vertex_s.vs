#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aColor;
layout (location = 1) in vec2 atexCoord;


uniform float size;
uniform float xAxis;
uniform mat4 transform;
uniform float yAxis;


out vec3 ourColor;
out vec2 TexCoord;



uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(size * aPos.x + xAxis, size * -aPos.y + yAxis, size * aPos.z, 1.0);
    ourColor = aColor;
    TexCoord = atexCoord;
}
