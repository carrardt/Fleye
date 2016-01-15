
void main(void)
{
	vec2 tc = var_TexCoord;
	vec4 MV =(texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y-step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y-step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y+step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y+step.y*0.25) ) )*0.25;

	vec4 MV4 = texture2D(tex_mv4, var_TexCoord );
	vec4 MV16 = texture2D(tex_mv16, var_TexCoord );

	vec2 mvec1 = vec2( MV.x-0.5 , MV.y-0.5 );
	vec2 mvec4 = vec2( MV4.x-0.5 , MV4.y-0.5 );
	vec2 mvec16 = vec2( MV16.x-0.5 , MV16.y-0.5 );

	vec2 mv = mvec1*MV.z + mvec4*4.0*MV4.z + mvec16*MV16.z*16.0;
	mv = normalize(mv);

	gl_FragColor.xy = MV.xy; // negative values are truncated, for debug
	gl_FragColor.z = MV.z;
	gl_FragColor.w = MV.w;
}
 
