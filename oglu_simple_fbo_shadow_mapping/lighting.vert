
uniform mat4 lightMatrix;

varying vec4 lightPos;
varying vec4 lightProj;

void main( void)
{


	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;




	lightProj= gl_TextureMatrix[0]* gl_Vertex;
	lightPos=gl_LightSource[0].position- gl_Vertex;

}