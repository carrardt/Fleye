
void main()
{
	vec2 tc = var_TexCoord;

	vec3 Sbl = texture2D(tex, vec2(tc.x-step.x, tc.y-step.y) ).xyz;
	vec3 Sb  = texture2D(tex, vec2(tc.x       , tc.y-step.y) ).xyz;
	vec3 Sbr = texture2D(tex, vec2(tc.x+step.x, tc.y-step.y) ).xyz;

	vec3 Sl = texture2D(tex, vec2(tc.x-step.x, tc.y) ).xyz;
	vec3 Sr = texture2D(tex, vec2(tc.x+step.x, tc.y) ).xyz;
	
	vec3 Sul = texture2D(tex, vec2(tc.x-step.x, tc.y+step.y) ).xyz;
	vec3 Su  = texture2D(tex, vec2(tc.x       , tc.y+step.y) ).xyz;
	vec3 Sur = texture2D(tex, vec2(tc.x+step.x, tc.y+step.y) ).xyz;

/*
	float d1 = length( Sr-Sl );
	float d2 = length( Su-Sb );
	float d3 = length( Sur-Sbl );
	float d4 = length( Sul-Sbr );
	float edgeDetect = (d1+d2+d3+d4)*0.25;
	gl_FragColor.xyz = vec3(edgeDetect,edgeDetect,edgeDetect);
*/
	vec3 d1 = abs( Sr-Sl );
	vec3 d2 = abs( Su-Sb );
	vec3 d3 = abs( Sur-Sbl );
	vec3 d4 = abs( Sul-Sbr );
	gl_FragColor.xyz = max( max(d1,d2) , max(d3,d4) )*2.0;

	gl_FragColor.w = 1.0;
}
