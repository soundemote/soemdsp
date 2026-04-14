const char* textureFragShaderString = R""(
#version 330

precision highp float;

uniform sampler2D tex;
uniform int blendMode;
in vec2 uv;
in vec2 pos;
out vec4 color;

void main()
{
    vec4 rgba = texture(tex, uv);
    switch (blendMode)
    {
    case 0:
    case 1:
        color = rgba;
        break;
    case 2:
    case 3:
    case 4:
        color = rgba * rgba.a;
        break;
    }
}
)"";
