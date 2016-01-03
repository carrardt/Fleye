
// object 1 : green objects
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

// object 2 : red hilights (i.e. laser dot)
float obj2Mask(vec3 p)
{
	float i2 = dot(p,p);
	float gbMax = max( p.y, p.z );
	float redDiff = p.x - gbMax;

	if( i2>0.33 && redDiff>0.25 ) return 1.0;
	else return 0.0;
}

