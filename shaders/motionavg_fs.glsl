vec3 chooseMV(vec3 A, vec3 B)
{
	if( A.z >= B.z ) return A;
	else return B;
}

void main(void)
{
	vec2 tc = var_TexCoord;
	/*
	vec4 MV =(texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y-step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y-step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y+step.y*0.25) )
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y+step.y*0.25) ) )*0.25;


	vec2 mvec1 = MV.xy; //*2.0 - vec2(1.0,1.0);
	vec2 mvec4 = MV4.xy; //*2.0 - vec2(1.0,1.0);
	vec2 mvec16 = MV16.xy; //*2.0 - vec2(1.0,1.0);
*/

	vec4 MVa = texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y-step.y*0.25) );
	vec4 MVb = texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y-step.y*0.25) );
	vec4 MVc = texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y+step.y*0.25) );
	vec4 MVd = texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y+step.y*0.25) );

	vec4 MV4 = texture2D(tex_mv4, var_TexCoord );
	vec4 MV16 = texture2D(tex_mv16, var_TexCoord );

	vec3 SelectedMV = (MVa.xyz + MVb.xyz + MVc.xyz + MVd.xyz)*0.25 ; //chooseMV( chooseMV(MVc.xyz,MVd.xyz) , chooseMV(MVa.xyz,MVb.xyz) );
	//SelectedMV = chooseMV( SelectedMV , MV4.xyz );
	//SelectedMV = chooseMV( SelectedMV , MV16.xyz );
	SelectedMV = MV4.xyz;
	
	/*if( MV4.z > l )
	{
		mv = mvec4;
		l = MV4.z;
	}
	if( MV16.z > l )
	{
		mv = mvec16;
		l = MV16.z;
	}*/
	
	vec2 mv = SelectedMV.xy;
	float l = SelectedMV.z;
	
	//mv = normalize(mv);
	//mv = mv*0.5 + vec2(0.5,0.5);

	gl_FragColor.xy = mv.xy; // negative values are truncated, for debug
	gl_FragColor.z = l;
	gl_FragColor.w = 1.0;
}
 
