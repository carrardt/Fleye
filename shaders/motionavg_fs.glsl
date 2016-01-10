
void main(void)
{
	vec2 tc = var_TexCoord;
	vec3 MV =(texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y-step.y*0.25) ).xyz
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y-step.y*0.25) ).xyz
			+ texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y+step.y*0.25) ).xyz
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y+step.y*0.25) ).xyz )*0.25;

	vec3 MV4 = texture2D(tex_mv4, var_TexCoord ).xyz;
	vec3 MV16 = texture2D(tex_mv16, var_TexCoord ).xyz;

	vec2 mv = MV.xy*MV.z + MV4.xy*4.0*MV4.z + MV16.xy*MV16.z*16.0;
	float c = (MV.z+MV4.z+MV16.z) ;
	mv = normalize(mv);

	gl_FragColor.xy = mv;
	gl_FragColor.z = c;
	gl_FragColor.w = 1.0;
}
 
