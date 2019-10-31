//-----------------------------------------------------------------------------
//           Name: dx9_hlsl_fx_simple.fx
//         Author: Kevin Harris
//  Last Modified: 04/15/04
//    Description: This effect file demonstrates how to write vertex and pixel
//                 shaders using Direct3D's High-Level Shading Language.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

float4x4 worldViewProj : WorldViewProjection; // This matrix will be loaded by the application

texture testTexture; // This texture will be loaded by the application

sampler Sampler = sampler_state
{
    Texture   = (testTexture);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

// Our sample application will send vertices 
// down the pipeline laid-out like this...

struct VS_INPUT
{
    float3 position	: POSITION;
	float2 texture0 : TEXCOORD0;
};

// Once the vertex shader is finished, it will 
// pass the vertices on to the pixel shader like this...

struct VS_OUTPUT
{
    float4 hposition : POSITION;
	float2 texture0  : TEXCOORD0;
    float4 color	 : COLOR0;
};

// And finally, the pixel shader will send a single 
// color value to the frame buffer like this...

struct PS_OUTPUT
{
	float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Simple Vertex Shader
//-----------------------------------------------------------------------------

VS_OUTPUT myvs( VS_INPUT IN )
{
    VS_OUTPUT OUT;

	OUT.hposition = mul( worldViewProj, float4(IN.position, 1) );

	OUT.color = float4( 1.0, 1.0, 1.0, 1.0 ); // Pass white as a default color
	//OUT.color = float4( 0.0, 1.0, 0.0, 1.0 ); // Pass green to test vertex shader

	OUT.texture0 = IN.texture0;

	return OUT;
}

//-----------------------------------------------------------------------------
// Simple Pixel Shader
//-----------------------------------------------------------------------------

PS_OUTPUT myps( VS_OUTPUT IN )
{
    PS_OUTPUT OUT;

	OUT.color = tex2D( Sampler, IN.texture0 ) * IN.color;

	// If you uncomment the next line, the color passed to us by our
    // vertex shader and the selected texture sampler will be ignored 
	// completely and all output to the frame buffer will be blue regardless.

	//OUT.color = float4( 0.0, 0.0, 1.0, 1.0);

    return OUT;
}

//-----------------------------------------------------------------------------
// Simple Effect (1 technique with 1 pass)
//-----------------------------------------------------------------------------

technique Technique0
{
    pass Pass0
    {
		Lighting = FALSE;

		Sampler[0] = (Sampler); // Needed by pixel shader

		VertexShader = compile vs_1_1 myvs();
		PixelShader  = compile ps_1_1 myps();
    }
}

