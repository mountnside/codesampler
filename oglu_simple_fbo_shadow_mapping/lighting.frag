
uniform sampler2D shadowMap;

varying vec4 lightPos;
varying vec4 lightProj;

// Use use another bitshift constant, notice that
// its the reciprocal of the other bit shift constant
// in the shadow.frag shader.
const vec3 bitShifts = vec3(1.0/(256.0*256.0), 1.0/256.0, 1);

void main(void)
{

	// Firstly, we get the 2D coords of the shadow map,
	// and get its value.
	vec3 pos = lightProj.xyz/lightProj.w;
	vec4 shadmap = texture2D(shadowMap,pos.xy );


	// Next we just have some constants, the bias is used to
	// 'push' the shadow back, to reduce 'Z-fighting' on the
	// shadow maps. The range is used to normalize the distance
	// between the fragment and the light, (just like we did
	// in shadow.frag, notise the value 100).
	const float bias = 0.1;
	const float range = 100.0;

	// We calculate the distance between the fragment and the light.
	// Then we apply the bias and the normalization by dividing
	// it by the range;
	float lightLen=length(lightPos);
	float lightDist=(lightLen-bias)/range;

	// Now we apply the bit shift, and add all of the components 
	// together. A dot product does this very cleanly.
	// This is all we need to do to convert the shadow map into
	// a distance value.
	float shad = dot(shadmap.xyz , bitShifts);
        
	// Perform the actual shadow map test. If shad is larger than
	// light dist, then the current fragment is outside of the shadow.
	float shadow = float(shad >lightDist);
	
	// NOTE: For show purposes, the shadow map is also drawn.
	// The only value we need is 'shadow'.
	// This is just to show you how freaky the unconverted
	// shadow map looks.
	gl_FragColor=shadmap *shadow ;

}