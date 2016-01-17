
void main()
{
	vec4 M = texture2D(tex_mv,var_TexCoord);
	vec3 C = texture2D(tex_camera,var_TexCoord).xyz;
	vec3 yuv = rgb_to_yuv( C );
	//yuv.x -= 0.5;
	//yuv.y += 0.5;
	//yuv.z += 0.5;
	//float Y = dot( C , vec3(0.299,0.587,0.114) );
	
	float edge = M.w;
	float mq = M.z;
	vec2 mv = M.xy;

	//mq = mq*(1.0-edge);


	gl_FragColor.xyz = yuv_to_rgb( vec3(mq,mv.x*mq,mv.y*mq) );
	gl_FragColor.w = 1.0;
}
