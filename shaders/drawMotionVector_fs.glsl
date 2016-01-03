
void main()
{
	vec4 Mvec = texture2D(tex_mv,var_TexCoord);
	float M = clamp(Mvec.x*32.0,0.0,1.0);
	vec3 C = texture2D(tex_camera,var_TexCoord).xyz;
	float Y = rgb_to_yuv(C).x; // greyscale value
	gl_FragColor.xyz = yuv_to_rgb( vec3(Y,M,M) );
	gl_FragColor.w = 1.0;
}
