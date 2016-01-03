
void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	//texcoord.y = 1.0 - texcoord.y;
#ifdef L2C_FILTER_TEX
	vec3 ftex =( texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x-step.x*0.5, texcoord.y+step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y-step.y*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+step.x*0.5, texcoord.y+step.y*0.5) ).xyz ) * 0.25;
#else
	vec3 ftex = texture2D(tex,texcoord).xyz;
#endif

	float gm = obj1Mask(ftex) * L2C_UNIT;
	float lm = obj2Mask(ftex) * L2C_UNIT;

	gl_FragColor = vec4(gm,gm,lm,lm);
}
