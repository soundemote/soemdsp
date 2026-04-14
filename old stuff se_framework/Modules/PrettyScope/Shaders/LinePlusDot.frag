const char* linePlusDotFragShaderString = R""(
#version 330

precision highp float;

uniform sampler2D aboveTexture;
uniform sampler2D belowTexture;
in vec2 uv;
in vec2 pos;
out vec4 color;

void main()
{
  vec4 abovePixel = texture(aboveTexture, uv);
  vec4 belowPixel = texture(belowTexture, uv);
  abovePixel.a = clamp(abovePixel.a, 0, 1);
  belowPixel.a = clamp(belowPixel.a, 0, 1);

  float alpha = abovePixel.a + belowPixel.a * (1 - abovePixel.a);
  if (alpha == 0)
  {
    color = vec4(0, 0, 0, 0);
    return;
  }
  vec3 rgb = (abovePixel.a * abovePixel.rgb + (1 - abovePixel.a) * belowPixel.a * belowPixel.rgb) / alpha;
  color = clamp(vec4(rgb, alpha), 0, 1);
}
)"";
