
void main(void)
{
	vec2 T0 = var_TexCoord;
	vec2 T1 = vec2( var_TexCoord.x , var_TexCoord.y+step.y );
	vec2 T2 = vec2( var_TexCoord.x , var_TexCoord.y+2.0*step.y );
	
	float m0 = compactL2Dist( T0, step2i );
	float m1 = compactL2Dist( T1, step2i );
	float m2 = compactL2Dist( T2, step2i );

	//vec4 C = texture2D( tex, T0 );
	// when using render buffer, we read back from offscreen display,
	// wich happens to pre-multiply alpha channel, so we have to set alpha
	// to 1.0 and we cannot use it together with other components.
	// nevertheless, offscreen displays are still much faster to read (through vc API)
	// than GL framebuffer (with glReadPixels)
	gl_FragColor = vec4( m0, m1, m2, 1.0 );
}
