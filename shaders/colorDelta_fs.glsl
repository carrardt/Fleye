void main()
{
	vec2 tc = var_TexCoord;
	
	vec3 Sl = texture2D(tex, vec2(tc.x-step.x, tc.y) ).xyz;
	vec3 Sr = texture2D(tex, vec2(tc.x+step.x, tc.y) ).xyz;
	vec3 Sbl = texture2D(tex, vec2(tc.x-step.x, tc.y-step.y) ).xyz;
	vec3 Sb  = texture2D(tex, vec2(tc.x       , tc.y-step.y) ).xyz;
	vec3 Sbr = texture2D(tex, vec2(tc.x+step.x, tc.y-step.y) ).xyz;
	vec3 Sul = texture2D(tex, vec2(tc.x-step.x, tc.y+step.y) ).xyz;
	vec3 Su  = texture2D(tex, vec2(tc.x       , tc.y+step.y) ).xyz;
	vec3 Sur = texture2D(tex, vec2(tc.x+step.x, tc.y+step.y) ).xyz;

	vec3 d1 = abs( Sr-Sl );
	vec3 d2 = abs( Su-Sb );
	vec3 d3 = abs( Sur-Sbl );
	vec3 d4 = abs( Sul-Sbr );

	gl_FragColor.xyz = max( max(d1,d2) , max(d3,d4) );
	gl_FragColor.w = 1.0;
}
