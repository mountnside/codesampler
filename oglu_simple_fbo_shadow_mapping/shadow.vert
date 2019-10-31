
// We will need to send only one var to the fragment shader.
varying vec3 vertPos;

void main(void)
{

	// Set and send OutPut data
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// This is just setting 'vertPos' to the vertex's position
	// in view space. This way, it will be easy to get the distance
	// between the light and the vert, as we assume the camera
	// is positioned at the light source.
	vertPos = vec4(gl_ModelViewMatrix * gl_Vertex).xyz;

	    

}