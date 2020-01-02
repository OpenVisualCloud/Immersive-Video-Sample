#version 150
in vec3 vPosition;
in vec2 aTexCoord;
out vec2 texCoord;

void main()
{
    gl_Position = vec4(vPosition,1.0);
    texCoord = aTexCoord;
}