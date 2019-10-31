float4x4 WorldIT;
float4x4 WorldViewProj;
float4x4 World;
float4x4 ViewI;
float3 LightPos = {20.0f,0,20.0f};
float3 LightColor = {1.0f, 1.0f, 1.0f};
float3 AmbiColor = {0.17f, 0.17f, 0.17f};
float3 WoodColor1 = {0.85f, 0.55f, 0.01f};
float3 WoodColor2 = {0.60f, 0.41f, 0.0f};
float Ks = 0.6;
float SpecExpon = 12.0;
float RingScale = 0.66;
float AmpScale = 0.7;
float PScale = 10;
float3 POffset = {-10.0f, -11.0f, 7.0f};

texture NoiseTex;

sampler3D NoiseSamp = sampler_state
{
    Texture = <NoiseTex>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = None;
};

/************* DATA STRUCTS **************/

struct appdata 
{
    float3 Position : POSITION;
    float4 UV       : TEXCOORD0;
    float4 Normal   : NORMAL;
};

struct vertexOutput
{
    float4 HPosition   : POSITION;
    float4 TexCoord    : TEXCOORD0;
    float3 LightVec    : TEXCOORD1;
    float3 WorldNormal : TEXCOORD2;
    float3 WoodPos     : TEXCOORD3;
    float3 WorldView   : TEXCOORD4;
};

/*********** vertex shader ******/

vertexOutput mainVS(appdata IN)
{
    vertexOutput OUT;
    OUT.WorldNormal = mul(IN.Normal, WorldIT).xyz;
    float4 Po = float4(IN.Position.xyz,1);
    float3 Pw = mul(Po, World).xyz;
    OUT.WoodPos = (PScale*Po) + POffset;
    OUT.LightVec = LightPos - Pw;
    OUT.TexCoord = IN.UV;
    OUT.WorldView = (ViewI[3].xyz - Pw);
    float4 hpos = mul(Po, WorldViewProj);
    OUT.HPosition = hpos;
    return OUT;
}

/********* pixel shader ********/

float4 woodPS(vertexOutput IN) : COLOR
{
    float3 Ln = normalize(IN.LightVec);
    float3 Nn = normalize(IN.WorldNormal);
    float3 Pwood = IN.WoodPos + (AmpScale * tex3D(NoiseSamp,IN.WoodPos.xyz/32.0).xyz);
    float r = RingScale * sqrt(dot(Pwood.yz,Pwood.yz));
    r = r + tex3D(NoiseSamp,r.xxx/32.0).x;
    r = r - floor(r);
    r = smoothstep(0.0, 0.8, r) - smoothstep(0.83,1.0,r);
    float3 dColor = lerp(WoodColor1,WoodColor2,r);
    float3 Vn = normalize(IN.WorldView);
    float3 Hn = normalize(Vn + Ln);
    float hdn = dot(Hn,Nn);
    float ldn = dot(Ln,Nn);
    float4 litV = lit(ldn,hdn,SpecExpon);
    float3 diffContrib = dColor * ((litV.y*LightColor) + AmbiColor);
    float3 specContrib = Ks * litV.z * LightColor;
    float3 result = diffContrib + specContrib;
    return float4(result,1);
}

technique wood
{
    pass p0
    {       
        VertexShader = compile vs_2_0 mainVS();
        ZEnable = true;
        ZWriteEnable = true;
        CullMode = None;
        PixelShader = compile ps_2_0 woodPS();
    }
}
