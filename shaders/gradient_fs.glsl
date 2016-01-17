//#define ENABLE_FILTER_TEX 1

vec4 lookup_Tex(vec2 t)
{
	vec4 C;
	C.xyz = texture2D(tex,t).xyz;
	C.w = length( C.xyz );
	return C;
}

void main()
{
	vec4 A = lookup_Tex(var_TexCoord);
	vec4 Ab = lookup_Tex( vec2(var_TexCoord.x,var_TexCoord.y-step.y) );
	vec4 Au = lookup_Tex( vec2(var_TexCoord.x,var_TexCoord.y+step.y) );
	vec4 Al = lookup_Tex( vec2(var_TexCoord.x-step.x,var_TexCoord.y) );
	vec4 Ar = lookup_Tex( vec2(var_TexCoord.x+step.x,var_TexCoord.y) );
	
	vec4 Ax = abs( ( (A-Al) + (Ar-A) ) *0.5 );
	vec4 Ay = abs( ( (A-Ab) + (Au-A) ) *0.5 );
	vec4 Ad = max(Ax,Ay);

	gl_FragColor = Ad;
}
