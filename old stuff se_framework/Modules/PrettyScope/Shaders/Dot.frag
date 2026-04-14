const char* dotFragShaderString = R""(
#version 330

#define TAUR 2.5066282746310002

precision highp float;

uniform float uSize;
uniform float tailFade;
uniform float uIntensity;
uniform float uBlur;
uniform vec4 brushColor;
uniform int brushProfile;
uniform int nSamples;
uniform vec2 ScreenSize;
in vec4 uvl;
out vec4 color;

float ScreenPixels(float pixels);
float Gaussian(vec3 uvl);
float QuadraticGradient(vec3 uvl);
float LinearGradient(vec3 uvl);
float PowerGradient(vec3 uvl);
float Flat(vec3 uvl);
float ControlledGradient(vec3 uvl);
float Taper(vec3 uvl, float lowest, float highest, float taper);

float alpha;

void main(void)
{
	vec3 xyz = uvl.xyz;
	switch (brushProfile)
  {
    case 0: alpha =                    Gaussian(xyz); break;
    case 1: alpha = Taper(xyz, -0.500, +0.999, -0.1); break;
    case 2: alpha =                        Flat(xyz); break;
  }

	alpha *= uIntensity;
	alpha *= smoothstep(0.0, tailFade, uvl.w);
	alpha = clamp(brushColor.a * alpha, 0, 1);

	color = vec4(brushColor.r, brushColor.g, brushColor.b, alpha);

	//float div = nSamples;
	//float factor = uvl.w/div;
	//color = 2*vec4(factor, 2*(factor - 0.5), -2*(factor - 0.5), alpha); //LENGHT-WISE COLOR
}
)"";
