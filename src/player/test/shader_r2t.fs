#version 150
in vec2 texCoord;
uniform sampler2D frameTex;
uniform sampler2D frameU;
uniform sampler2D frameV;
uniform int isNV12;
void main()
{
    //bool isyuv420 = true;
    if(isNV12 == 0) //yuv420
    {
    vec4 c = vec4((texture(frameTex, texCoord).r - 16.0/255.0) * 1.164);
    vec4 U = vec4(texture(frameU, texCoord).r - 128.0/255.0);
    vec4 V = vec4(texture(frameV, texCoord).r - 128.0/255.0);
    c += V * vec4(1.596, -0.813, 0, 0);
    c += U * vec4(0, -0.392, 2.017, 0);
    c.a = 1.0;
    gl_FragColor = c;
    }
    else //nv12
    {
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture(frameTex, texCoord).r -16./256.;
    yuv.y = texture(frameU, texCoord).r - 128./256.;
    yuv.z = texture(frameU, texCoord).g - 128./256.;
    rgb = mat3( 1,       1,         1,
                0,       -0.39465,  2.03211,
                1.13983, -0.58060,  0) * yuv;
    gl_FragColor = vec4(rgb, 1);
    }
}
