#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform float size;
uniform float xAxis;
out vec3 ourColor;

void main()
{
    gl_Position = vec4(size * aPos.x + xAxis, size * -aPos.y, size * aPos.z, 1.0);
    ourColor = aColor;
}
