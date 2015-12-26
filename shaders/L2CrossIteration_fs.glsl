#define UNIT (1.0/32.0)

/*
 *   w
 * x * y
 *   z
 */
float incrementL2( float c, float nbh )
{
	if( nbh>0.0 && nbh==c ) c+=UNIT;
	return c;
}

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec4 C = texture2D( tex, texcoord );

	float tx = texcoord.x;
	float ty = texcoord.y;

	float Tx_p = tx + step2i.x;
	float Ty_p = ty + step2i.y;
		
	if( Tx_p<1.0 )
	{
		vec4 nbh1 = texture2D( tex, vec2(Tx_p,ty) );
		C.x = incrementL2( C.x, nbh1.x);
		C.z = incrementL2( C.z, nbh1.z);
	}

	if( Ty_p<1.0 )
	{
		vec4 nbh2 = texture2D( tex, vec2(tx,Ty_p) );
		C.y = incrementL2( C.y, nbh2.y);
		C.w = incrementL2( C.w, nbh2.w);
	}

	gl_FragColor = C;
}
