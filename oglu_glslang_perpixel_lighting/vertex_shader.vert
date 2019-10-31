//The normal for computing our lighting.
varying vec3 normal;

//Position of the vertex
varying vec4 pos;

void main( void )
{
	//Get the normal
	normal = gl_NormalMatrix * gl_Normal;

	//Get the vertex position
	pos = gl_ModelViewMatrix * gl_Vertex;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
    //gl_FrontColor = gl_Color;
}
