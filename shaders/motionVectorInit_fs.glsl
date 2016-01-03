//#define ENABLE_FILTER_TEX 1

vec3 lookup_A(vec2 t)
{
	return texture2D(tex,t).xyz;
}

vec3 lookup_B(vec2 t)
{
	return texture2D(texPrev,t).xyz;
}

void main()
{
	vec3 A = lookup_A(var_TexCoord);
	vec3 Ab = lookup_A( vec2(var_TexCoord.x,var_TexCoord.y-step.y) );
	vec3 Au = lookup_A( vec2(var_TexCoord.x,var_TexCoord.y+step.y) );
	vec3 Al = lookup_A( vec2(var_TexCoord.x-step.x,var_TexCoord.y) );
	vec3 Ar = lookup_A( vec2(var_TexCoord.x+step.x,var_TexCoord.y) );
	vec3 Ax = abs( ( (A-Al) + (Ar-A) ) *0.5 );
	vec3 Ay = abs( ( (A-Ab) + (Au-A) ) *0.5 );
	vec3 Ad = max(Ax,Ay);
	
	vec3 B = lookup_B(var_TexCoord);
	vec3 Bb = lookup_B( vec2(var_TexCoord.x,var_TexCoord.y-step.y) );
	vec3 Bu = lookup_B( vec2(var_TexCoord.x,var_TexCoord.y+step.y) );
	vec3 Bl = lookup_B( vec2(var_TexCoord.x-step.x,var_TexCoord.y) );
	vec3 Br = lookup_B( vec2(var_TexCoord.x+step.x,var_TexCoord.y) );
	vec3 Bx = abs( ( (B-Bl) + (Br-B) ) * 0.5 );
	vec3 By = abs( ( (B-Bb) + (Bu-B) ) * 0.5 );
	vec3 Bd = max(Bx,By);

	vec3 GradDiff = Bd - Ad;
	//vec3 Diff = B - A ;

	gl_FragColor = vec4( dot(GradDiff,GradDiff), 0.0, 0.0, 1.0 );
}
