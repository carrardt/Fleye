float computeSimilarity( vec3 ref, vec2 tc, vec2 N )
{
	vec3 Diff = texture2D(tex, tc + N*step ).xyz - ref;
	Diff = vec3( 1.0-abs(Diff.x), 1.0-abs(Diff.y), 1.0-abs(Diff.z) ) ;
	return dot(Diff,Diff);
}

void main(void)
{
	vec3 A = texture2D(texPrev, var_TexCoord ).xyz;

	//vec3 B = texture2D(tex, texcoord ).xyz;
	vec2 V1 = vec2(-1.0,-1.0);
	vec2 V2 = vec2(0.0,-1.0);
	vec2 V3 = vec2(1.0,-1.0);
	vec2 V4 = vec2(1.0,0.0);
	vec2 V5 = vec2(1.0,1.0);
	vec2 V6 = vec2(0.0,1.0);
	vec2 V7 = vec2(-1.0,1.0);
	vec2 V8 = vec2(-1.0,0.0);

	float S1 = computeSimilarity( A, var_TexCoord, V1 );
	float S2 = computeSimilarity( A, var_TexCoord, V2 );
	float S3 = computeSimilarity( A, var_TexCoord, V3 );
	float S4 = computeSimilarity( A, var_TexCoord, V4 );
	float S5 = computeSimilarity( A, var_TexCoord, V5 );
	float S6 = computeSimilarity( A, var_TexCoord, V6 );
	float S7 = computeSimilarity( A, var_TexCoord, V7 );
	float S8 = computeSimilarity( A, var_TexCoord, V8 );

	vec2 mv = S1*V1 + S2*V2 + S3*V3 + S4*V4 + S5*V5 + S6*V6 + S7*V7 + S8*V8;
	mv = normalize(mv);

	gl_FragColor.xy = mv*0.5+vec2(0.5,0.5);
	gl_FragColor.z = 0.0; //(S1+S2+S3+S4+S5+S6+S7+S8)*0.015625;
	gl_FragColor.w = 1.0;
}
 
