void main()
{
	vec2 t = var_TexCoord;
	vec3 N2 =( texture2D(tex, vec2(t.x-step.x, t.y-step.y) ).xyz
			 + texture2D(tex, vec2(t.x-step.x, t.y+step.y) ).xyz
			 + texture2D(tex, vec2(t.x+step.x, t.y-step.y) ).xyz
			 + texture2D(tex, vec2(t.x+step.x, t.y+step.y) ).xyz ) * 0.25;

	vec3 N =( texture2D(tex, vec2(t.x-step.x*0.5, t.y-step.y*0.5) ).xyz
			+ texture2D(tex, vec2(t.x-step.x*0.5, t.y+step.y*0.5) ).xyz
			+ texture2D(tex, vec2(t.x+step.x*0.5, t.y-step.y*0.5) ).xyz
			+ texture2D(tex, vec2(t.x+step.x*0.5, t.y+step.y*0.5) ).xyz ) * 0.25;
	

	vec3 C = texture2D(tex,t).xyz;
	
	vec3 F = C*0.5 + N*0.3 + N2*0.2;

	gl_FragColor = vec4( F , 1.0 );
}
