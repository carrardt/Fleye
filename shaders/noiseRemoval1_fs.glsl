void main()
{
	vec2 tc = var_TexCoord;
	
	vec3 Sl4 = texture2D(tex, vec2(tc.x-step.x*4.0, tc.y) ).xyz;
	vec3 Sl3 = texture2D(tex, vec2(tc.x-step.x*3.0, tc.y) ).xyz;
	vec3 Sl2 = texture2D(tex, vec2(tc.x-step.x*2.0, tc.y) ).xyz;
	vec3 Sl1 = texture2D(tex, vec2(tc.x-step.x, tc.y) ).xyz;
	vec3 S = texture2D(tex, tc ).xyz;
	vec3 Sr1 = texture2D(tex, vec2(tc.x+step.x, tc.y) ).xyz;
	vec3 Sr2 = texture2D(tex, vec2(tc.x+step.x*2.0, tc.y) ).xyz;
	vec3 Sr3 = texture2D(tex, vec2(tc.x+step.x*3.0, tc.y) ).xyz;
	vec3 Sr4 = texture2D(tex, vec2(tc.x+step.x*4.0, tc.y) ).xyz;

	vec3 hGauss = Sl4*0.0162162162 + Sl3*0.0540540541 + Sl2*0.1216216216 + Sl1*0.1945945946
				+ S * 0.2270270270
				+ Sr1*0.1945945946 + Sr2*0.1216216216 + Sr3*0.0540540541 + Sr4*0.0162162162;
	
	vec3 Sbl = texture2D(tex, vec2(tc.x-step.x, tc.y-step.y) ).xyz;
	vec3 Sb  = texture2D(tex, vec2(tc.x       , tc.y-step.y) ).xyz;
	vec3 Sbr = texture2D(tex, vec2(tc.x+step.x, tc.y-step.y) ).xyz;
	
	vec3 Sul = texture2D(tex, vec2(tc.x-step.x, tc.y+step.y) ).xyz;
	vec3 Su  = texture2D(tex, vec2(tc.x       , tc.y+step.y) ).xyz;
	vec3 Sur = texture2D(tex, vec2(tc.x+step.x, tc.y+step.y) ).xyz;

	float d1 = length( Sr1-Sl1 );
	float d2 = length( Su-Sb );
	float d3 = length( Sur-Sbl );
	float d4 = length( Sul-Sbr );
	float edgeDetect = (d1+d2+d3+d4)*0.25;

	gl_FragColor.xyz = hGauss;
	gl_FragColor.w = edgeDetect;
}
