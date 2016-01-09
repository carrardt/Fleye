// this version assumes that input texture is linear filtered, thus needs less texture fetches

void main()
{
	vec4 S1 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y-step.y*0.25)); 

	vec4 S2 = texture2D(tex,vec2(var_TexCoord.x-step.x*0.25,var_TexCoord.y+step.y*0.25)); 

	vec4 S3 = texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y-step.y*0.25)); 

	vec4 S4 = texture2D(tex,vec2(var_TexCoord.x+step.x*0.25,var_TexCoord.y+step.y*0.25)); 

	gl_FragColor = ( S1 + S2 + S3 + S4 ) * 0.25;
}
