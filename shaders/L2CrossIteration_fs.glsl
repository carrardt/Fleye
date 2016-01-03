
void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec4 C = texture2D( tex, texcoord );
	gl_FragColor = l2CrossIteration(C,texcoord,step2i);
}
