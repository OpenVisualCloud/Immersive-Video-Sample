#version 150
in vec3 vertex;
in vec2 texCoord0;
uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
out vec2 texCoord;
void main()
{
    gl_Position = uProjMatrix * uViewMatrix *  vec4(vertex, 1.0);
    texCoord = texCoord0;
}