//-------------------------------------------------------------------------------
//           Name: deferred.fx
//         Author: Nikita Kindt (nk47@bk.ru)
//  Last Modified: 17/06/06
//-------------------------------------------------------------------------------

float4x3 c_mWorld;
float4x3 c_mView;
float4x3 c_mViewInverse;
float4x4 c_mProjection;
float4x4 c_mViewProjection;

// light parameters
float3 c_vLightDir;
float4 c_vLightPos;
float4 c_vLightDiffuse;
float4 c_vLightSpecular;

// material parameters
float4 c_vMaterialDiffuse;
float4 c_vMaterialSpecular;
float c_fSpecularPower;

// textures
texture2D c_tDiffuseMap;
texture2D c_tSceneMaterialMap;
texture2D c_tSceneNormalMap;
texture2D c_tScenePositionXYMap;
texture2D c_tScenePositionZMap;


sampler DiffuseSampler = sampler_state
{
	Texture = <c_tDiffuseMap>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

// rendertargets
sampler SceneMaterialSampler = sampler_state
{
	Texture = <c_tSceneMaterialMap>;
	MagFilter = Point;
	MinFilter = Point;
};

sampler SceneNormalSampler = sampler_state
{
	Texture = <c_tSceneNormalMap>;
	MagFilter = Point;
	MinFilter = Point;
};

sampler ScenePositionXYSampler = sampler_state
{
	Texture = <c_tScenePositionXYMap>;
	MagFilter = Point;
	MinFilter = Point;
};

sampler ScenePositionZSampler = sampler_state
{
	Texture = <c_tScenePositionZMap>;
	MagFilter = Point;
	MinFilter = Point;
};



struct VS_INPUT_BUILD
{
    float3  vPos            : POSITION0;
    float2  vTex0           : TEXCOORD0;
    float3  vNrm            : NORMAL0;
    float3  vTan            : TANGENT0;
    float3  vBin            : BINORMAL0;
};

struct VS_OUTPUT_BUILD
{
	float4	vPos			: POSITION0;
    float2  vTex0           : TEXCOORD0;
	float3	vWorldPos		: TEXCOORD1;
	float3	vWorldNrm		: TEXCOORD2;
};

struct PS_OUTPUT_BUILD
{
	float4	vMaterial		: COLOR0;
	float4	vWorldNrm		: COLOR1;
	float4	vWorldPosXY		: COLOR2;
	float4	vWorldPosZ		: COLOR3;
};

VS_OUTPUT_BUILD vsBuild(VS_INPUT_BUILD i)
{
	VS_OUTPUT_BUILD o;

	o.vWorldPos = mul(float4(i.vPos, 1), c_mWorld);
	o.vPos = mul(float4(o.vWorldPos, 1), c_mViewProjection);	
	o.vTex0 = i.vTex0;	
	o.vWorldNrm = normalize(mul(float4(i.vNrm, 0), c_mWorld));
	
	return o;
};

// psBuild()
// put geometry data into render targets
PS_OUTPUT_BUILD psBuild(VS_OUTPUT_BUILD i) : COLOR0
{
	PS_OUTPUT_BUILD o;

	// material
	float4 vDiffuseMaterial = tex2D(DiffuseSampler, i.vTex0);

	o.vMaterial.rgb = vDiffuseMaterial;
	o.vMaterial.a = 1.0;
	
	// convert normal to texture space [-1;+1] -> [0;1]
	o.vWorldNrm.xyz = i.vWorldNrm * 0.5 + 0.5;
	o.vWorldNrm.w = 0.0;
	
	// position
	o.vWorldPosXY = float4(i.vWorldPos.xy, 0, 0);
	o.vWorldPosZ = float4(i.vWorldPos.z, 0, 0, 0);

	return o;
};



struct PS_INPUT_LIGHT
{
	float2 vTex0		: TEXCOORD0;
};

// psLighting()
// uses data from textures (previous render targets)
float4 psLighting(PS_INPUT_LIGHT i) : COLOR0
{
	float3 vDiffuseMaterial = tex2D(SceneMaterialSampler, i.vTex0).rgb;
	float3 vSpecularMaterial = tex2D(SceneMaterialSampler, i.vTex0).a;
	
	// normals are stored in texture space [0,1] -> convert them back to [-1,+1] range
	float3 vWorldNrm = (tex2D(SceneNormalSampler, i.vTex0) - 0.5) * 2;

	float3 vWorldPos;
	vWorldPos.xy = tex2D(ScenePositionXYSampler, i.vTex0).xy;
	vWorldPos.z = tex2D(ScenePositionZSampler, i.vTex0).x;
	float3 vLightDir = normalize(c_vLightPos - vWorldPos);
	float3 vEyeVec = normalize(c_mViewInverse[3].xyz - vWorldPos);	
	float3 vDiffuseIntensity = dot(vLightDir, vWorldNrm);
	float3 vSpecularIntensity = pow(max(0, dot(vEyeVec, reflect(-vLightDir, vWorldNrm))), c_fSpecularPower);

	float4 color;
	color.rgb = vDiffuseIntensity * c_vLightDiffuse.xyz * vDiffuseMaterial + 
				vSpecularIntensity * c_vLightSpecular.xyz * vSpecularMaterial;
	color.a = 1.0;
	
	// here we add color to show how lighting pass affects the scene
	color.rgb += i.vTex0.rgr * 0.5;
	
	return color;
};


technique buildPass
{
	pass p0
	{
		VertexShader = compile vs_1_1 vsBuild();
		PixelShader = compile ps_2_0 psBuild();
		
		CullMode = ccw;
		FillMode = solid;
		Zenable = true;
		ZWriteEnable = true;
		ZFunc = less;
		StencilEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;		
		ColorWriteEnable = red | green | blue;
	}
}

technique lightPass
{
    pass p0
	{
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psLighting();
		
		CullMode = none;
		FillMode = solid;
		Zenable = false;
		StencilEnable = false;		
		AlphaBlendEnable = true;
		Srcblend = One;
		Destblend = One;
		AlphaTestEnable = false;		
		ColorWriteEnable = red | green | blue;
	}
};
