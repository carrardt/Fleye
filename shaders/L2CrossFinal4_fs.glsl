
void main(void)
{
	//vec2 texcoord = normalizedWindowCoord();
	vec2 T0 = var_TexCoord;
	vec2 T1 = vec2( var_TexCoord.x , var_TexCoord.y+step.y );
	vec2 T2 = vec2( var_TexCoord.x , var_TexCoord.y+2.0*step.y );
	vec2 T3 = vec2( var_TexCoord.x , var_TexCoord.y+3.0*step.y );

	float m0 = compactL2Dist( T0, step2i );
	float m1 = compactL2Dist( T1, step2i );
	float m2 = compactL2Dist( T2, step2i );
	float m3 = compactL2Dist( T3, step2i );
	gl_FragColor = vec4( m0, m1, m2, m3 );
}
