const char* colormapFragShaderString = R""(
#version 330

precision highp float;

uniform sampler2D originalColor;
uniform sampler1D colorMap;
uniform int blendMode;
uniform float colorMapCorrection;
in vec2 uv;
out vec4 color;

float brightness_alpha_blend(vec4 color)
{
    return (color.r+color.g+color.b)/3 * color.a;
}

float brightness(vec4 color)
{
    return (color.r+color.g+color.b)/3;
    //return 0.3f * color.r + 0.59f * color.g + 0.11f * color.b;
}


float colorMapCorrectionFunc()
{
    float resolution = textureSize(colorMap, 0);
    return (resolution - 0.5f) / resolution;
}

void main()
{

  //vec4 oldColor = texture(originalColor, uv);
  //float resolution = textureSize(colorMap, 0);
  //color = texture(colorMap, oldColor.a);

  vec4 oldColor = texture(originalColor, uv);

  switch (blendMode)
  {
  case 0:
  case 1:
  case 2:
  case 4:
      color = texture(colorMap, brightness(oldColor) * colorMapCorrectionFunc());
      break;
  case 3:
      color = texture(colorMap, brightness_alpha_blend(oldColor) * colorMapCorrectionFunc());
      break;
  }
}
)"";
