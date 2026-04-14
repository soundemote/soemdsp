const char* dotVertShaderString = R""(
#version 330

precision highp float;

uniform float uSize;
uniform vec2 ScreenSize;
uniform vec2 ScaleFactor;
in vec2 aStart;
in float aIdx;
out vec4 uvl;

void main()
{
  float idx = mod(aIdx, 4.0);
  float tang = (floor(idx / 2.0) - 0.5) * 2.0;
  float side = (mod(idx, 2.0) - 0.5)*2.0;

  uvl.xy = vec2(tang, side);
  uvl.z = 1.0;
  uvl.w = floor(aIdx / 4.0 + 0.5);

  gl_Position = vec4((aStart + uvl.xy * uSize) * ScaleFactor, 0.0, 1.0);
}
)"";
