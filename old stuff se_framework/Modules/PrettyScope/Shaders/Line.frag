const char* lineFragShaderString = R""(
#version 330

#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951

precision highp float;
uniform float tailFade;
uniform float uSize;
uniform float LenFactor;
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

float erf(float x)
{
  float s = sign(x), a = abs(x);
  x = 1.0 + (0.278393 + (0.230389 + (0.000972 + 0.078108 * a) * a) * a) * a;
  x *= x;
  return s - s / (x * x);
}

float Physical(vec3 uvl)
{
    float len = uvl.z;
    vec2 xy = vec2((len / 2.0 + uSize)*uvl.x + len / 2.0, uSize*uvl.y);
    //vec2 xy = uvl.xy;
    float alpha;
    float sigma = uSize / 4.0;

    if (len < EPS)
    {
        // If the beam segment is too short, just calculate intensity at the position.
        alpha = exp(-dot(xy, xy) / (2.0*sigma*sigma)) / 2.0 * sigma;
    }
    else
    {
        // Otherwise, use analytical integral for accumulated intensity.
        alpha = erf(xy.x/SQRT2/sigma) - erf((xy.x-len)/SQRT2/sigma);
        alpha *= exp(-xy.y*xy.y / (2.0*sigma*sigma)) / 2.0 * sigma;
    }
    return alpha;
}


void main(void)
{
  vec3 xyz = uvl.xyz;
  float lenScale;
  if (uvl.z > EPS)
  {
    // converting the coordinates so they can be interpreted as circular
    float scale = 1 - uSize / (uvl.z / 2 + uSize);
    xyz.x = sign(uvl.x) * max((abs(uvl.x) - scale) / (1 - scale), 0);
    lenScale = 4.0 / uvl.z;
  }
  else
    lenScale = 100.0;

  float alpha;
  switch (brushProfile)
  {
    case 0: alpha =                Physical(uvl.xyz); alpha *= mix(200.0, lenScale, LenFactor)*3; break;
    case 1: alpha = Taper(xyz, 0.850, 0.999, -0.200); alpha *= mix(200.0, lenScale, LenFactor)*.003; break;
    case 2: alpha =                        Flat(xyz); alpha *= mix(500.0, lenScale, LenFactor)*.004; break;
  }

	alpha *= smoothstep(0.0, tailFade, uvl.w) * uIntensity;
  //if (uvl.z > EPS && LenFactor > 0.5)
  //  alpha *= 4.0 / uvl.z;
  float dimming = smoothstep(0.002, 0.005, uSize);
  alpha *= dimming *= dimming;
  alpha = clamp(alpha, 0, 1);

    color = vec4(brushColor.r, brushColor.g, brushColor.b, alpha);
  
  //float div = nSamples;
 // float factor = uvl.w/div;
 // color = vec4(1.0, 0.0,  0.0, alpha); //LENGHT-WISE COLOR
}
)"";
