const char* lineVertShaderString = R""(
#version 330

#define EPS 1E-6

precision highp float;

uniform float uSize;
uniform vec2 ScreenSize;
uniform vec2 ScaleFactor;
in vec2 aStart, aEnd;
in float aIdx;
out vec4 uvl;

void main()
{
  float tang;
  vec2 current;
  // All points in quad contain the same data:
  // segment start point and segment end point.
  // We determine point position from it's index.
  float idx = mod(aIdx, 4.0);
  if (idx >= 2.0)
  {
    current = aEnd;
    tang = 1.0;
  }
  else
  {
    current = aStart;
    tang = -1.0;
  }
	float side = (mod(idx, 2.0) - 0.5)*2.0;
  //
  vec2 dir = aEnd - aStart;
  uvl.z = length(dir);
  if (uvl.z > EPS)
    dir = dir / uvl.z;
  else
	{
    // If the segment is too short draw a square;
    dir = vec2(1.0, 0.0);
		//uvl.z = 1.0;
	}

	// extrude quad edges slightly
	float size;
	if (uSize > EPS)
	{
		size = uSize + 0.005;
		uvl.xy = vec2(tang * (uvl.z / 2.0 + size) / (uvl.z / 2.0 + uSize), side * size / uSize);
	}
	else
	{
		size = 0.005;
		uvl.xy = vec2(tang / EPS, side / EPS);
	}

	//float size = uSize;
	//uvl.xy = vec2(tang, side);
	uvl.w = floor(aIdx / 4.0 + 0.5);

  vec2 norm = vec2(-dir.y, dir.x);

  gl_Position = vec4((current + (tang * dir + norm * side) * size) * ScaleFactor, 0.0, 1.0);
}
)"";
