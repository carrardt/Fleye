
void main()
{
	vec4 Mvec = texture2D(tex_mv,var_TexCoord);
	vec3 C = texture2D(tex_camera,var_TexCoord).xyz;
	float Y = dot( C , vec3(0.299,0.587,0.114) );
	gl_FragColor.xyz = Mvec.xyz;
	gl_FragColor.w = 1.0;
}
