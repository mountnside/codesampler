varying vec3 vertPos;

void main(void)
{

    // The hardest part to understand is how we will store
    // the depth of the fragment.
    // This will be done in kind of a pseudo-bitshift
    // fashion, ie the red,green and blue components will
    // each store 8bits of the 24bit depth.
    // NOTE: Both 16bit or 32bit can be done in this fashion
    // with no speed gain or loss, however 24bit seems to
    // be enough for most depth buffers, so we will use that.


    // Firstly, we create the constant 'bit' vectors.
    // 'bitSh' is the bit shift, designed to 'shift' the 
    // depth value so that only a certain part lies within
    // the current 8bit component.
    // The second is the bit mask, designed to cull all numbers
    // below the 8bit component. Any value above will be culled by
    // the 'fract()' function.
    const vec3 bitSh = vec3(	256*256,	256,			1);
    const vec3 bitMsk = vec3(	0,		1.0/256.0, 	1.0/256.0);
    

    // We will need to calculate and store the distance between the
    // light and the fragment. 
    // NOTE: This value needs to be normalized, this can usually be
    // done by dividing the length by the lights range, but for simplicity here
    // we are dividing it by 100, which is the 'zFar' value set in 'gluPerspective()'.
    float dist = length(vertPos)/100.0;
    vec3 comp;
    

    // Now, we apply the bit shift, the commented code is what is
    // happening under the hood. If you dont understand bit shifting
    // or masking, just trust me on this. It will move a different section
    // of the 'distance' over the 0.0f-1.0f range of each component, so that
    // a 24bit float can be stored in 3*8bit floats.

    //comp.x=dist*(256*256);
    //comp.y=dist*256;
    //comp.z=dist;

    comp=dist*bitSh;


    // The next part (again, commented code shows what is happening)
    // is simply culling all unwanted 'bits'...all we want is a value
    // between 0.0f and 1.0f, with a precision of 1.0f/256.0f.
    // If the precision is better (1.0f/512.0f for example) the value
    // may be rounded off and we will get a bad value. And of course if the
    // value is above 1.0f the value will be clamped, which
    // again will produce a bad value.
    
    //comp.x=fract(comp.x);
    //comp.y=fract(comp.y)-comp.x/256;
    //comp.z=fract(comp.z)-comp.y/256;

    comp=fract(comp);
    comp-=comp.xxy*bitMsk;


    // For a simple one-texture projectsion spot light, this is perhaps not 
    // the best method (GL_DEPTH_COMPONENT should produce nicer results), but for cube
    // shadow mapping, this is the best way.

    // This also removes the need for floating point
    // buffers (GL_RGBA_32F) or for single component rendering (GL_LUMINANCE).
    // Which in some cases can be a plus for compatability.
    
    gl_FragColor=vec4(comp,1);

}