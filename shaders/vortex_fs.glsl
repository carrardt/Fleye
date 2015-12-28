
void main(void)
{
	vec2 center = vec2(0.5,0.5);
	vec2 p = var_TexCoord - center;
	float theta = dot(p,p)*10.0;
	float SinTheta = sin(theta);
	float CosTheta = cos(theta);
	vec2 t = vec2( CosTheta*p.x - SinTheta*p.x , SinTheta*p.y + CosTheta*p.y ) + center;
	gl_FragColor = texture2D(tex, t );
}
