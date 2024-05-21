#version 330 core
out vec4 Bloom;

in vec3 ourColor;
in vec2 TexCoord;


uniform sampler2D texture0;
uniform sampler2D texture1;


void main()
{
    Bloom = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2);
}
