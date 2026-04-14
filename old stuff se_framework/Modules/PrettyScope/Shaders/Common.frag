const char* commonFragShaderString = R""(
#version 330

#define TAU 6.283185307179586
#define PI 3.1415926535897932384626433832795
#define SQR(X) ((X) * (X))

precision highp float;

uniform float uSize;
uniform float uBlur;
uniform vec2 ScreenSize;

float map(float s, float a1, float a2, float b1, float b2)
{
  return b1 + (s-a1)*(b2-b1)/(a2-a1);
}
float ScreenPixels(float pixels)
{
	return 2.0 * pixels / (uSize * min(ScreenSize.x, ScreenSize.y));
}

float Gaussian(vec3 uvl)
{
	float sigma = mix(0.1, 0.2, uBlur);
  return exp(-dot(uvl.xy, uvl.xy) / (2 * SQR(sigma))) / (TAU * SQR(sigma));
}

float QuadraticGradient(vec3 uvl)
{
  return (1.0 - dot(uvl.xy, uvl.xy)) * 0.75;
}

float ControlledGradient(vec3 uvl)
{
	float v = length(uvl.xy);
	if (v > 1) return 0;
	float a = tan(mix(0.5 * PI, 1.5 * PI, uBlur));
	float b = tan(mix(0.5 * PI, 1.5 * PI, clamp(0.5, 0.0001, 0.9999)));
	v /= 12 / (a - b + 6);
	return (1 - (a * v + (3 - 2 * a - b) * v * v + (a + b - 2) * v * v * v));
}

float LinearGradient(vec3 uvl)
{
  return 1.0 - length(uvl.xy);
}

float PowerGradient(vec3 uvl)
{
	float power = max(mix(0, 1.5, uBlur), 0.00005);
	return (1.0 - pow(dot(uvl.xy, uvl.xy), power / 2)) * ((power + 1) / (2.0 * power));
}

float Taper(float v, float t)
{
  return (t*v-v)/(2*t*v-t-1);
}

float Taper(vec3 uvl, float lowest, float highest, float taper)
{
  float v = 1.0 - length(uvl.xy);
  float t = map(Taper(uBlur, taper), 0, 1, lowest, highest);
  return Taper(v, t);
}

float Flat(vec3 uvl)
{
  return smoothstep(0.0, max(ScreenPixels(uBlur*100), ScreenPixels(3)), 1.0 - length(uvl.xy));
	//return step(0, 1.0 - length(uvl.xy));
}
)"";
