#define UNIT (1.0/32.0)

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

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec3 A =( texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y+step.y*0.5) ).xyz
			+ texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y+step.y*0.5) ).xyz ) * 0.25;
	vec3 B =( texture2D(texPrev, vec2(texcoord.x-step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x-step.x*0.5, texcoord.y+step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x+step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x+step.x*0.5, texcoord.y+step.y*0.5) ).xyz ) * 0.25;

	B = B - A;
	float motion = dot(B,B);
	float motionMask = clamp( sign(motion-0.1) , 0.0 , 1.0 );

	float gm = greenMask(A)*motionMask * UNIT;
	float lm = 0.0; //motionMask * UNIT;

	gl_FragColor = vec4(gm,gm,lm,lm);
}
