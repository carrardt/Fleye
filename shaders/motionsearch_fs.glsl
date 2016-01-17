float computeSimilarity( vec4 A, vec4 B )
{
	vec3 d = vec3(1.0,1.0,1.0) - abs( B.xyz - A.xyz );
	return dot(d,d);
}

void main(void)
{
	vec2 tc = var_TexCoord;
	vec4 A = texture2D(texPrev, tc );
	vec4 B = texture2D(tex, tc );
	
	float E  = B.w; // edge detection from noise filter
	
	vec2 V1 = vec2(-1.0,-1.0);
	vec2 V2 = vec2(0.0,-1.0);
	vec2 V3 = vec2(1.0,-1.0);
	vec2 V4 = vec2(1.0,0.0);
	vec2 V5 = vec2(1.0,1.0);
	vec2 V6 = vec2(0.0,1.0);
	vec2 V7 = vec2(-1.0,1.0);
	vec2 V8 = vec2(-1.0,0.0);

	float S0 = computeSimilarity( A, B );
	float S1 = computeSimilarity( A, texture2D(tex,tc+V1*step) );
	float S2 = computeSimilarity( A, texture2D(tex,tc+V2*step) );
	float S3 = computeSimilarity( A, texture2D(tex,tc+V3*step) );
	float S4 = computeSimilarity( A, texture2D(tex,tc+V4*step) );
	float S5 = computeSimilarity( A, texture2D(tex,tc+V5*step) );
	float S6 = computeSimilarity( A, texture2D(tex,tc+V6*step) );
	float S7 = computeSimilarity( A, texture2D(tex,tc+V7*step) );
	float S8 = computeSimilarity( A, texture2D(tex,tc+V8*step) );
	
	/*float MinS = min( min( min(S1,S2) , min(S3,S4) ) , min( min(S5,S6) , min(S7,S8) ) );
	float MaxS = max( max( max(S1,S2) , max(S3,S4) ) , max( max(S5,S6) , max(S7,S8) ) );
	float SDiff = MaxS - MinS;*/

	vec2 mv = (1.0-S0) * ( S1*V1 + S2*V2 + S3*V3 + S4*V4 + S5*V5 + S6*V6 + S7*V7 + S8*V8 );
	float l = length(mv)*0.125;
	mv = normalize(mv);

	gl_FragColor.xy = mv*0.5 + vec2(0.5,0.5);
	gl_FragColor.z = l;
	gl_FragColor.w = E; 
}

