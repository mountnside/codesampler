#define DEPTH_THRESHOLD 0.008

varying vec2 vTexCoord;

uniform sampler2D uColorTexture;
uniform sampler2D uDepthTexture;
uniform float uOffset;

//The offset.
vec2 Samples[12] = {
	{-0.326212, -0.405805},
	{-0.840144, -0.073580},
	{-0.695914, 0.457137},
	{-0.203345, 0.620716},
	{0.962340, -0.194983},
	{0.473434, -0.480026},
	{0.519456, 0.767022},
	{0.185461, -0.893124},
	{0.507431, 0.064425},
	{0.896420, 0.412458},
	{-0.321940, -0.932615},
	{-0.791559, -0.597705}
};

void main() 
{
	float Color = texture2D(uColorTexture, vTexCoord).x;
	float Depth = texture2D(uDepthTexture, vTexCoord).x;
 	//float depth_threshold = (Depth + DEPTH_THRESHOLD * projection.z.z) / projection.z.w

	int Num = 1.0; 

	//Just loop samples.
	for(int i = 0; i < 12; i++)
	{
		vec2 Tex = vTexCoord + uOffset * Samples[i];
		if(abs(texture2D(uDepthTexture, Tex).x - Depth) < DEPTH_THRESHOLD)
		{
			Color += texture2D(uColorTexture, Tex).x;
			Num = Num + 1.0;
		}
	}
			
	//The average.
	Color /= Num;

	gl_FragColor = vec4(Color, Color, Color, 1.0);
}
