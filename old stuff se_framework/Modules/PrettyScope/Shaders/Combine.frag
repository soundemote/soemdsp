const char* combineFragShaderString = R""(
#version 330

precision highp float;

uniform sampler2D oldPixel;
uniform sampler2D newPixel;

uniform int blendMode;
uniform float decaySpeed;

in vec2 uv;
in vec2 pos;
out vec4 color;

vec4 alpha_blend(vec4 oldColor, vec4 newColor)
{
	newColor.a = clamp(newColor.a, 0, 1);
	oldColor.a = clamp(oldColor.a, 0, 1);
	float outA = newColor.a + oldColor.a * (1 - newColor.a);
	if (outA == 0)
	    return vec4(0, 0, 0, 0);
    return vec4(( newColor.a * newColor.rgb + oldColor.a * (1 - newColor.a) * oldColor.rgb ) / outA, outA);
}

vec4 soft_saturation(vec4 oldColor, vec4 newColor, float decayFactor)
{
    decayFactor += 1.0;
		return vec4((oldColor.rgb + newColor.rgb*decayFactor) / (decayFactor + newColor.rgb), 1);
    //return vec4 (oldColor + newColor*decayFactor) / (decayFactor + newColor);
    // original accumulate function:
    // (oldColor + newColor) / (1 + newColor)
}

vec4 soft_saturation_alpha_blend(vec4 oldColor, vec4 newColor, float decayFactor)
{
    decayFactor += 1.0;
    newColor *= newColor.a;
    return vec4((oldColor.rgb + newColor.rgb*decayFactor) / (decayFactor + newColor.rgb), 1);
}

// x is the number of frames
// f(x) = color - decayFactor * x
vec4 linear_saturation(vec4 oldColor, vec4 newColor, float decayFactor)
{
    oldColor.a -= decayFactor;
    return alpha_blend(oldColor, newColor);
}

// f(x) = color * (1 - decayFactor) ^ x
vec4 exponential_decay(vec4 oldColor, vec4 newColor, float decayFactor)
{
    oldColor.a = (1 - decayFactor) * oldColor.a;
    oldColor = oldColor * (1 - decayFactor);
    return alpha_blend(oldColor, newColor);
}

void main()
{
    vec4 oldColor = texture(oldPixel, uv);
    vec4 newColor = texture(newPixel, uv);

    ///*Linear Light?*/
    //newColor.rgb = (oldColor.rgb + newColor.rgb)/(1+decaySpeed);
    //newColor.a = 1.0;
    //color = clamp(newColor, 0,1);

    ///*Custom blend*/
    //newColor.rgb *= newColor.a;
    //oldColor.a -= decaySpeed;
    //oldColor.rgb *= oldColor.a;
    //color = clamp(vec4(oldColor.rgb + newColor.rgb, 1.0),0,1);

    switch (blendMode)
    {
    case 0:
    case 1:
        color = soft_saturation(oldColor, newColor, decaySpeed);
        break;
    case 2:
    case 3:
    case 4:
        color = linear_saturation(oldColor, newColor, decaySpeed);
        break;
    }
}
)"";
