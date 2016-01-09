
void main(void)
{
	vec2 tc = var_TexCoord;
	vec2 MV =(texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y-step.y*0.25) ).xy
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y-step.y*0.25) ).xy
			+ texture2D(tex_mv, vec2(tc.x-step.x*0.25,tc.y+step.y*0.25) ).xy
			+ texture2D(tex_mv, vec2(tc.x+step.x*0.25,tc.y+step.y*0.25) ).xy )*0.25;

	vec2 MV4 = texture2D(tex_mv4, var_TexCoord ).xy;
	vec2 MV16 = texture2D(tex_mv16, var_TexCoord ).xy;

	vec2 mv = MV+MV4*2.0+MV16*4.0;

	gl_FragColor.xy = mv*0.5+vec2(0.5,0.5);
	gl_FragColor.z = 0.0; //(S1+S2+S3+S4+S5+S6+S7+S8)*0.015625;
	gl_FragColor.w = 1.0;
}
 
