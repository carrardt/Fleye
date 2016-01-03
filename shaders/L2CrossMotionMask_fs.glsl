
// object 1 : greenish areas
float obj1Mask(vec3 p)
{
	const float greenThreshold = 0.667;

	float rbMax = max( p.x, p.z );
	float rbMin = min( p.x, p.z );
	float greenDiff = (p.y-rbMin);
	float greenRatio = (p.y - rbMax) / (p.y-rbMin);

	if( greenDiff>0.05 && greenRatio>greenThreshold ) return 1.0;
	else return 0.0;
}

// object 2 : moving areas
float obj2Mask(vec3 A)
{
#ifdef L2C_FILTER_TEX
	vec3 B =( texture2D(texPrev, vec2(texcoord.x-step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x-step.x*0.5, texcoord.y+step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x+step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			+ texture2D(texPrev, vec2(texcoord.x+step.x*0.5, texcoord.y+step.y*0.5) ).xyz ) * 0.25;
#else
	vec3 B = texture2D(texPrev,texcoord).xyz;
#endif
	B = B - A;
	float motion = dot(B,B);
	return clamp( sign(motion-0.1) , 0.0 , 1.0 );
}
