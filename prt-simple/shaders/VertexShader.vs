varying vec3 N;
varying vec3 v;
varying vec2 vTexCoord;

void main(void)
{
	v = vec3(gl_ModelViewMatrix * gl_Vertex);       
	//vec4 Normal = gl_ModelViewMatrix * vec4(gl_Normal, 0);	
	//N = normalize(Normal.xyz);	
	N = normalize(gl_NormalMatrix * gl_Normal);

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vTexCoord = gl_MultiTexCoord0.xy;
}