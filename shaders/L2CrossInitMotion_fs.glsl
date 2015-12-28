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
	vec3 A = texture2D(tex, texcoord ).xyz;
	vec3 B = texture2D(texPrev, texcoord ).xyz;
	B = B - A;
	float motion = dot(B,B);
	float motionMask = clamp( sign(motion-0.05) , 0.0 , 1.0 );

	float gm = greenMask(A) * UNIT;
	float lm = motionMask * UNIT;

	gl_FragColor = vec4(gm,gm,lm,lm);
}
