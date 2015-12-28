vec3 hue_to_rgb(float x)
{
	if( x < 0.3333 )
	{
		x = x * 3.0;
		return vec3(1.0-x,x,0.0);		
	}
	else if( x < 0.6666 )
	{
		x = (x-0.3333) * 3.0;
		return vec3(0.0,1.0-x,x);
	}
	else
	{
		x = (x-0.6666) * 3.0;
		return vec3(x,0.0,1.0-x);
	}
}

vec3 rgb_to_yuv( vec3 c )
{
	return vec3( 0.299*c.x+0.587*c.y+0.114*c.z, 0.500*c.x-0.419*c.y-0.081*c.z, -0.169*c.x-0.331*c.y+0.500*c.z );
}

vec3 yuv_to_rgb( vec3 yuv )
{
	return vec3( yuv.x + 1.403*yuv.z , yuv.x - 0.344*yuv.y - 0.714*yuv.z , yuv.x + 1.770*yuv.y );
}
