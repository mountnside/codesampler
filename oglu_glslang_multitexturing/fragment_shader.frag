
vec4 finalcolor;

uniform sampler2D Texture1;			// Our Texture no.1
uniform sampler2D Texture2;			// Our Texture no.2

uniform bool multitexture;

void main( void )
{
	finalcolor = texture2D(Texture1,gl_TexCoord[0].xy);  // Add texel color

	if(multitexture)
	finalcolor *= texture2D(Texture2,gl_TexCoord[0].xy);  // Add texel color again

	gl_FragColor = finalcolor;

}														  
														  
														  
