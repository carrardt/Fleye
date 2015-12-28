varying vec4 var_Color;
varying vec2 var_TexCoord;

uniform vec2 step;
uniform vec2 size;
uniform float iter;
uniform float iter2i;
uniform vec2 step2i;

// based on gl_FragCoord
vec2 normalizedWindowCoord()
{
	vec2 normCoord = (gl_FragCoord.xy+vec2(0.5,0.5)) * step;
	//normCoord.y = 1.0 - normCoord.y;
	return normCoord;
}

vec2 pixelWindowCoord()
{
	return gl_FragCoord.xy;
}
