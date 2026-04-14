const char* basicVertShaderString = R""(
#version 330

precision highp float;

in vec2 position;
in vec2 texCoord;
out vec2 uv;
out vec2 pos;

void main()
{
  uv = texCoord;
  pos = position;
  gl_Position = vec4(pos, 0, 1);
}
)"";
