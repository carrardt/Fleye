#define UNIT (1.0/32.0)
#define FILTER_TEX 1

float greenMask(vec3 p)
{
	const float greenThreshold = 0.667;

	float rbMax = max( p.x, p.z );
	float rbMin = min( p.x, p.z );
	float greenDiff = (p.y-rbMin);
	float greenRatio = (p.y - rbMax) / (p.y-rbMin);

	if( greenDiff>0.05 && greenRatio>greenThreshold ) return 1.0;
	else return 0.0;
}

float laserMask(vec3 p)
{
	float i2 = dot(p,p);
	float gbMax = max( p.y, p.z );
	float redDiff = p.x - gbMax;

	if( i2>0.33 && redDiff>0.25 ) return 1.0;
	else return 0.0;
}

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	//texcoord.y = 1.0 - texcoord.y;
#ifdef FILTER_TEX
	vec3 ftex =( texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y+step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y+step.y*0.5) ).xyz ) * 0.25;
#else
	vec3 ftex = texture2D(tex,texcoord).xyz;
#endif
	float gm = greenMask(ftex) * UNIT;
	float lm = laserMask(ftex) * UNIT;

	gl_FragColor = vec4(gm,gm,lm,lm);
}
