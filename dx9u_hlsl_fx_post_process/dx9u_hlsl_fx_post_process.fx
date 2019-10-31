//-----------------------------------------------------------------------------
//           Name: dx9u_hlsl_fx_post_process.fx
//         Author: Michael Guerrero
//  Last Modified: 04/27/06
//    Description: This effect file demonstrates how to write vertex and pixel
//                 shaders using Direct3D's High-Level Shading Language combined 
//                 with a simple post processing edge detection effect.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

const float du;
const float dv;

// Post Processing Texture
texture offscreenTexture;

sampler2D PostProcessSampler = sampler_state
{
	Texture   = <offscreenTexture>;
	MinFilter = None;
	MagFilter = None;
	MipFilter = None;
	AddressU  = Wrap;
	AddressV  = Wrap;
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

struct VS_OUTPUT8
{
	float4 position    : POSITION;
	float2 textures[8] : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Sobel_VS - Edge detection using the sobel mask
//-----------------------------------------------------------------------------

VS_OUTPUT8 Sobel_VS( float4 position: POSITION, half2 tc: TEXCOORD0 )
{
	VS_OUTPUT8 o;

	// We have 8 texture coordinates and we start
	// each one from the original coordinate
	for (int i = 0; i < 8; ++i)
		o.textures[i] = tc;

	o.textures[0] += half2(-du, -dv);   // Top Left
	o.textures[1] += half2(  0, -dv);   // Up
	o.textures[2] += half2( du, -dv);	// Top Right
	o.textures[3] += half2(-du,   0);	// Left
	o.textures[4] += half2( du,   0);	// Right
	o.textures[5] += half2(-du,  dv);	// Bottom Left
	o.textures[6] += half2(  0,  dv);	// Bottom
	o.textures[7] += half2( du,  dv);	// Bottom Right

	// This vertex now has 8 texture coordinates to sample from
	// Before they get to the pixel shader, they're values
	// will be automatically interpolated per pixel

	o.position = position;
	
	return o;
}

// -----------------------------------------
// Sobel Mask Edge Detection
// - Here we apply edge detection using the sobel mask
// - Notice that the weights add up to 0
// -----------------------------------------
// Gx = -1 0 1	Gy = 1 2 1
//		-2 0 2		 0 0 0
//		-1 0 1		-1-2-1
// -----------------------------------------	
float4 Sobel_PS(half2 textures[8]: TEXCOORD): COLOR
{
	half4 g00 = tex2D(PostProcessSampler, textures[0]);  // Top Left
	half4 g01 = tex2D(PostProcessSampler, textures[1]);  // Up
	half4 g02 = tex2D(PostProcessSampler, textures[2]);  // Top Right
	half4 g10 = tex2D(PostProcessSampler, textures[3]);  // Left
	half4 g12 = tex2D(PostProcessSampler, textures[4]);  // Right
	half4 g20 = tex2D(PostProcessSampler, textures[5]);  // Bottom Left
	half4 g21 = tex2D(PostProcessSampler, textures[6]);  // Bottom
	half4 g22 = tex2D(PostProcessSampler, textures[7]);  // Bottom Right

	half4 Gx = -(g00 + 2 * g10 + g20) + (g02 + 2 * g12 + g22);
	half4 Gy = -(g00 + 2 * g01 + g02) + (g20 + 2 * g21 + g22);

	half4 color = (Gx * Gx) + (Gy + Gy);

	return sqrt(color);
}

//-----------------------------------------------------------------------------
// Simple post process Technique using the Sobel Mask Edge Detection
// (1 technique with 1 pass)
//-----------------------------------------------------------------------------

technique TechniqueSobel
{
	Pass P0
	{	
		Lighting     = false;
		ZWriteEnable = false;
		
		VertexShader = compile vs_2_0 Sobel_VS();
		PixelShader  = compile ps_2_0 Sobel_PS();
	}
}
