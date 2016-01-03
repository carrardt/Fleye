#define L2C_FILTER_TEX 1
#define L2C_UNIT (1.0/32.0)

float incrementL2( float c, float nbh )
{
	if( nbh>0.0 && nbh==c ) c += L2C_UNIT;
	return c;
}

vec4 l2CrossIteration( vec4 C, vec2 tc, vec2 st )
{
	float tx = tc.x;
	float ty = tc.y;

	float Tx_p = tx + st.x;
	float Ty_p = ty + st.y;
		
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
	
	return C;
}

float compactL2Dist( vec2 tc, vec2 st )
{
	vec4 C = texture2D( tex, tc );
	C = l2CrossIteration(C, tc, st);

	float Ar = C.x;
	float Au = C.y;
	float Br = C.z;
	float Bu = C.w;
		
	float Am = max(Ar,Au);
	float Bm = max(Br,Bu);
	float m=0.0;
	
	if( Am >= Bm )
	{
		m = Am;
	}
	else
	{
		m = 0.5 + Bm;
	}
	return m;
}
