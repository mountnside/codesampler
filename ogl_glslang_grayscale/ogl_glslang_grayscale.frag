uniform sampler2D testTexture;
uniform vec4 grayScaleWeights;

void main( void )
{
    // Fetch the regular RGB texel color from the texture
    vec4 texelColor = texture2D( testTexture, gl_TexCoord[0].xy );

    //
    // Converting to grayscale:
    //
    // Converting an image to grayscale is done by taking a weighted average of
    // the red, green and blue color components. The standard weights for this 
    // type of conversion are (0.30, 0.59, 0.11). Therefore, the gray component 
    // or luminance that we need to compute can be defined as a luminance 
    // filter like so:
    //
    // luminance = 0.30*R + 0.59*G + 0.11*B
    //
    // If we think of our RGB colors as vectors, we can see that this 
    // calculation is actually just a dot product.
    //

    vec4 scaledColor = texelColor * grayScaleWeights;
    float luminance = scaledColor.r + scaledColor.g + scaledColor.b;

    //
    // Now, even though we want a grayscale color, we still need to output a  
    // three component RGB color. To do this, we'll set each color component  
    // to be the same luminance. This is becuase every value on the grayscale  
    // is always an equal mixture of red, green and blue. For example, pure 
    // white is (1.0,1.0,1.0), pure black is (0.0,0.0,0.0), and a perfectly 
    // middle gray would be (0.5,0.5,0.5).
    //

    gl_FragColor = vec4( luminance, luminance, luminance, texelColor.a );
}
