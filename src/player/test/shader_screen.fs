#version 150
uniform sampler2D frameTex_screen;
in vec2 texCoord;
out vec4 fragColor;
void main()
{
    fragColor = texture(frameTex_screen, texCoord);
}