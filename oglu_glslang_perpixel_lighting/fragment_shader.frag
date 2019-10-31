// Inputs
varying vec3 normal;
varying vec4 pos;


vec3 lightDir;
vec4 diffuse = vec4(2.0,2.0,2.0,1.0);
vec4 ambient = vec4(0.01,0.01,0.01,1.0);
float distance;  // Distance between light position and fragment
vec4 finalcolor;
float NdotL;     // Dot product of normal and lightDir

float attenuation;


uniform sampler2D testTexture;

void main( void )
{

	normal=normalize(normal);

	// gl_LightSource[0].position is used to access position 
	// of GL_LIGHT0
	lightDir = normalize(vec3(gl_LightSource[0].position-pos));

	distance = length(vec3(gl_LightSource[0].position-pos));

	NdotL = max(dot(normal,lightDir),0.0);

	// Attenuation -- just like the Red Book does it 

	attenuation = 1.00 / (gl_LightSource[0].constantAttenuation +
					gl_LightSource[0].linearAttenuation * distance +
					gl_LightSource[0].quadraticAttenuation * distance * distance);


	finalcolor = texture2D(testTexture,gl_TexCoord[0].xy);  // Add texel color to vertex color

	finalcolor *= attenuation * (diffuse * NdotL + ambient); // put all the lighting together


	gl_FragColor = finalcolor; 
}
