uniform sampler2D myTexture;

varying vec2 vTexCoord;
varying vec3 N;
varying vec3 v;
uniform float EyeX, EyeY, EyeZ;

vec4 PhongModel(vec3 Pos, vec3 Normal)
{
	vec4 Color = gl_FrontMaterial.emission;
	//vec3 g_vEyePos = gl_ModelViewMatrixInverse * vec4(EyeX, EyeY, EyeZ, 1);
	vec3 g_vEyePos = vec3(0,0,0);
	vec3 EyeToPos = normalize(Pos - g_vEyePos);	
	vec3 LightToPos = vec3(0,0,0), Reflected = vec3(0,0,0);
	vec3 LightPos = vec3(0,0,0);
	float RawDiffuseIntensity, DiffuseIntensity = 0.0, Spec = 0.0;
	int i = 0;	
	
	for (i = 0; i < gl_MaxLights; i++)
	{
		LightPos = gl_LightSource[i].position.xyz;
		LightToPos = normalize(Pos - LightPos);
		
		// Compute the diffuse component
		DiffuseIntensity = clamp(dot(-LightToPos, Normal), 0.0, 1.0);			
		
		// Compute the specular component
		//Reflected = normalize(LightToPos - 2.0*dot(Normal, LightToPos) * Normal);
		Reflected = normalize(reflect(LightToPos, Normal));
		Spec = clamp(dot(-Reflected, EyeToPos), 0.0, 1.0);
		Spec = pow(Spec, 10.0);

		Color = Color + (gl_LightSource[i].diffuse * DiffuseIntensity + gl_LightSource[i].specular*Spec)/gl_LightSource[i].constantAttenuation;
		//Color = Color + (gl_LightSource[i].diffuse * DiffuseIntensity)/gl_LightSource[i].constantAttenuation;
	}	
	
	return Color;
}

void main (void)  
{  
    gl_FragColor = PhongModel(v, N);
}    
