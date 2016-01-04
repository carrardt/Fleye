
void main()
{
	vec4 S1 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.75,var_TexCoord.y-step.y*0.75))
			+ texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y-step.y*0.75)) 
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y-step.y*0.75)) 
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.75,var_TexCoord.y-step.y*0.75)); 

	vec4 S2 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.75,var_TexCoord.y-step.y*0.25))
			+ texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y-step.y*0.25))
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y-step.y*0.25)) 
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.75,var_TexCoord.y-step.y*0.25)); 

	vec4 S3 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.75,var_TexCoord.y+step.y*0.25)) 
			+ texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y+step.y*0.25)) 
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y+step.y*0.25)) 
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.75,var_TexCoord.y+step.y*0.25)); 

	vec4 S4 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.75,var_TexCoord.y+step.y*0.75))
			+ texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y+step.y*0.75))
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y+step.y*0.75))
			+ texture2D(tex,vec2(var_TexCoord.x+step.x*0.75,var_TexCoord.y+step.y*0.75)); 

	gl_FragColor = ( S1*0.25 + S2*0.25 + S3*0.25 + S4*0.25 ) * 0.25;
}
