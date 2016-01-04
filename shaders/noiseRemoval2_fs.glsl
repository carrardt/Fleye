
void main()
{
	vec2 tc = var_TexCoord;
	
	vec3 Sl4 = texture2D(tex_nr1, vec2(tc.x, tc.y-step.y*4.0) ).xyz;
	vec3 Sl3 = texture2D(tex_nr1, vec2(tc.x, tc.y-step.y*3.0) ).xyz;
	vec3 Sl2 = texture2D(tex_nr1, vec2(tc.x, tc.y-step.y*2.0) ).xyz;
	vec3 Sl1 = texture2D(tex_nr1, vec2(tc.x, tc.y-step.y) ).xyz;
	vec4 S = texture2D(tex, tc );
	vec3 Sr1 = texture2D(tex_nr1, vec2(tc.x, tc.y+step.y) ).xyz;
	vec3 Sr2 = texture2D(tex_nr1, vec2(tc.x, tc.y+step.y*2.0) ).xyz;
	vec3 Sr3 = texture2D(tex_nr1, vec2(tc.x, tc.y+step.y*3.0) ).xyz;
	vec3 Sr4 = texture2D(tex_nr1, vec2(tc.x, tc.y+step.y*4.0) ).xyz;

	vec3 vGauss = Sl4*0.0162162162 + Sl3*0.0540540541 + Sl2*0.1216216216 + Sl1*0.1945945946
				+ S.xyz * 0.2270270270
				+ Sr1*0.1945945946 + Sr2*0.1216216216 + Sr3*0.0540540541 + Sr4*0.0162162162;
	
	float edgeDetect = clamp( S.w*0.25 ,0.0, 1.0);

	vec3 Original = texture2D(tex,tc).xyz;

	gl_FragColor.xyz = edgeDetect*Original + (1.0-edgeDetect)*vGauss;
	gl_FragColor.w = edgeDetect; // might be useful
}
