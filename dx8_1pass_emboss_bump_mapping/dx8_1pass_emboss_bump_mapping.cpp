//-----------------------------------------------------------------------------
//           Name: dx8_1pass_emboss_bump_mapping.cpp
//         Author: Kevin Harris
//  Last Modified: 03/25/05
//    Description: This sample demonstrates how to perform one-pass emboss 
//                 bump mapping with Direct3D.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <d3d8.h>
#include <d3dx8.h>
#include <mmsystem.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D8             g_pD3D          = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE8      g_baseTexture   = NULL;
LPDIRECT3DTEXTURE8      g_bumpTexture   = NULL;
D3DLIGHT8               g_pLight0;
LPD3DXMESH              g_pSphereMesh   = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

bool  g_bEmbossBumpMap  = true;
bool  g_bEmbossOnly     = false;
bool  g_bMoveLightAbout = true;
float g_fEmbossFactor   = 1.0f;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

struct Vertex
{
    float x , y, z;
    float nx, ny, nz;
    float tu, tv;
    float tu2, tv2;

    enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2
	};
};

const int NUM_VERTICES = 4;

Vertex g_quadVertices[NUM_VERTICES] =
{
//     x      y     z     nx    ny     nz     tu   tv     tu2  tv2
    {-1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,0.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f,0.0f,  1.0f,0.0f },
	{-1.0f,-1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,1.0f,  0.0f,1.0f },
    { 1.0f,-1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f,1.0f,  1.0f,1.0f }
};

D3DXVECTOR3 g_vTangents[NUM_VERTICES];
D3DXVECTOR3 g_vBiNormals[NUM_VERTICES];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void shutDown(void);
void render(void);

D3DXVECTOR3 computeTangentVector(Vertex pVtxA,Vertex pVtxB,Vertex pVtxC);
void computeTangentsAndBinormals(void);
void shiftTextureCoordinates(void);
void renderQuadWithEmbossBumpMapping(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "Direct3D (DX8) - One-Pass Emboss Bump Mapping",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		{
			g_dCurTime     = timeGetTime();
			g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
			g_dLastTime    = g_dCurTime;

		    render();
		}
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
	
    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

                case VK_F1:
		            g_bEmbossBumpMap = !g_bEmbossBumpMap;
		        break;

                case VK_F2:
                    g_bEmbossOnly = !g_bEmbossOnly;
                    if( g_bEmbossOnly == true )
                        g_bEmbossBumpMap = true;
                break;

                case VK_F3:
                    g_bMoveLightAbout = !g_bMoveLightAbout;
		        break;

                case VK_F4:
                    g_fEmbossFactor += 0.1f;
                break;

                case VK_F5:
		            g_fEmbossFactor -= 0.1f;
		        break;
			}
		}
        break;

		case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bMousing )
			{
				g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
			}
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture( void )
{
	D3DXCreateTextureFromFile( g_pd3dDevice, "woodfloor.tga", &g_baseTexture );
    //D3DXCreateTextureFromFile( g_pd3dDevice, "test.tga", &g_baseTexture );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
						  D3DDEVTYPE_HAL, 
						  //D3DDEVTYPE_REF,
						  g_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						  &d3dpp, &g_pd3dDevice );

	loadTexture();

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    //
    // Set up a material, a single light source, and turn on some global 
    // ambient lighting...
    //

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    D3DMATERIAL8 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
    mtrl.Diffuse.r = 1.0f;
    mtrl.Diffuse.g = 1.0f;
    mtrl.Diffuse.b = 1.0f;
    mtrl.Diffuse.a = 1.0f;
    mtrl.Ambient.r = 1.0f;
    mtrl.Ambient.g = 1.0f;
    mtrl.Ambient.b = 1.0f;
    mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &mtrl );

    //g_pLight0.Type = D3DLIGHT_POINT;
	g_pLight0.Type = D3DLIGHT_DIRECTIONAL;
	g_pLight0.Position  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    g_pLight0.Diffuse.r = 1.0f;
    g_pLight0.Diffuse.g = 1.0f;
    g_pLight0.Diffuse.b = 1.0f;
    g_pLight0.Diffuse.a = 1.0f;
    g_pLight0.Ambient.r = 1.0f;
    g_pLight0.Ambient.g = 1.0f;
    g_pLight0.Ambient.b = 1.0f;
    g_pLight0.Ambient.a = 1.0f;
    g_pd3dDevice->SetLight( 0, &g_pLight0 );
    g_pd3dDevice->LightEnable( 0, TRUE );
	
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.25, 0.25, 0.25, 1.0 ) );

	// Create a sphere mesh to represent a point light.
    D3DXCreateSphere(g_pd3dDevice, 0.05f, 8, 8, &g_pSphereMesh, NULL);
	
    //
    // For each vertex, create a tangent vector and binormal
    //

    computeTangentsAndBinormals();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pSphereMesh != NULL )
        g_pSphereMesh->Release();

    if( g_baseTexture != NULL ) 
        g_baseTexture->Release();

    if( g_bumpTexture != NULL ) 
        g_bumpTexture->Release();

    if( g_pVertexBuffer != NULL ) 
        g_pVertexBuffer->Release(); 

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: computeTangentVector()
// Desc: To find a tangent that heads in the direction of +tv, find
//       the components of both vectors on the tangent surface, and add a 
//       linear combination of the two projections that head in the +tv 
//       direction
//-----------------------------------------------------------------------------
D3DXVECTOR3 computeTangentVector( Vertex pVtxA, Vertex pVtxB, Vertex pVtxC )
{
	D3DXVECTOR3 vAB = D3DXVECTOR3(pVtxB.x, pVtxB.y, pVtxB.z) - D3DXVECTOR3(pVtxA.x, pVtxA.y, pVtxA.z);
	D3DXVECTOR3 vAC = D3DXVECTOR3(pVtxC.x, pVtxC.y, pVtxC.z) - D3DXVECTOR3(pVtxA.x, pVtxA.y, pVtxA.z);
	D3DXVECTOR3 nA  = D3DXVECTOR3(pVtxA.nx, pVtxA.ny, pVtxA.nz);

    // Components of vectors to neghboring vertices that are orthogonal to the
    // vertex normal
    D3DXVECTOR3 vProjAB = vAB - ( D3DXVec3Dot( &nA, &vAB ) * nA );
    D3DXVECTOR3 vProjAC = vAC - ( D3DXVec3Dot( &nA, &vAC ) * nA );

    // tu texture coordinate differences
    FLOAT duAB = pVtxB.tu - pVtxA.tu;
    FLOAT duAC = pVtxC.tu - pVtxA.tu;

	// tv texture coordinate differences
    FLOAT dvAB = pVtxB.tv - pVtxA.tv;
    FLOAT dvAC = pVtxC.tv - pVtxA.tv;

    if( (duAC * dvAB) > (duAB * dvAC) )
    {
        duAC = -duAC;
        duAB = -duAB;
    }
    
    D3DXVECTOR3 vTangent = (duAC * vProjAB) - (duAB*vProjAC);
    D3DXVec3Normalize( &vTangent, &vTangent );
    return vTangent;
}

//-----------------------------------------------------------------------------
// Name: computeTangentsAndBinormals
// Desc: For each vertex, create a tangent vector and binormal
//-----------------------------------------------------------------------------
void computeTangentsAndBinormals( void )
{	
	Vertex *pVertices = NULL;
    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );

    //
    // Even though our simple quad isn't being rendered via an index buffer. 
    // It's useful to think of our geomtery as being indexed when it comes time
    // to calulate tangents and binormals. This helps to average each tangent 
    // vector across all trianlges that make use it, which inturn, produces  
    // much smoother results.
    // 
    // Our quad uses D3DPT_TRIANGLESTRIP to render, which means that our 
    // quad's four vertices will be indexed like so to create the two 
    // triangles required to create the quad:
    //
    // Tri #1 = 0, 1, 2
    // Tri #2 = 2, 3, 1
    //
    // See the diagrams for D3DPT_TRIANGLESTRIP in the DirectX help files if 
    // this isn't clear.
    //

    const int nNumIndices = 6;
	int indices[nNumIndices] = { 0,1,2,  2,3,1 };

    //
	// For every triangle or face, use the indices to find the vertices 
	// that make it up. Then, compute the tangent vector for each one,
	// averaging whenever a vertex happens to be shared amongst triangles.
    //

    for( int i = 0; i < nNumIndices; i += 3 )
    {
		int a = indices[i+0];
        int b = indices[i+1];
        int c = indices[i+2];

        // We use += becuase we want to average the tangent vectors with 
        // neigboring triangles that share vertices.
		g_vTangents[a] += computeTangentVector( pVertices[a], pVertices[b], pVertices[c] );
		g_vTangents[b] += computeTangentVector( pVertices[b], pVertices[a], pVertices[c] );
		g_vTangents[c] += computeTangentVector( pVertices[c], pVertices[a], pVertices[b] );
	}

    //
    // Normalize each tangent vector and create a binormal to pair with it...
    //

    for( i = 0; i < NUM_VERTICES; ++i )
    {
        D3DXVec3Normalize( &g_vTangents[i], &g_vTangents[i] );

		D3DXVec3Cross( &g_vBiNormals[i],
			           &D3DXVECTOR3( pVertices[i].nx, pVertices[i].ny, pVertices[i].nz ),
					   &g_vTangents[i] );
    }

	g_pVertexBuffer->Unlock();
}

//-----------------------------------------------------------------------------
// Name: shiftTextureCoordinates()
// Desc: 
//-----------------------------------------------------------------------------
void shiftTextureCoordinates( void )
{
    Vertex *pv = NULL;
    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pv, 0 );

    // Get the world matrix and inverse it
	D3DXMATRIX inverseWorld;
 	g_pd3dDevice->GetTransform( D3DTS_WORLD, &inverseWorld );
	D3DXMatrixInverse( &inverseWorld, NULL, &inverseWorld);
    
	// Calculate the current light position in object space
    D3DXVECTOR4 vTemp;
    D3DXVECTOR3 vLightsPosition;
	D3DXVec3Transform( &vTemp, (D3DXVECTOR3*)&g_pLight0.Position, &inverseWorld );
    vLightsPosition.x = vTemp.x;
    vLightsPosition.y = vTemp.y;
    vLightsPosition.z = vTemp.z;

	// Get the dimensions of the texture so we can shift the tex coords properly.
    D3DSURFACE_DESC d3dsd;
    g_baseTexture->GetLevelDesc( 0, &d3dsd );

	//
    // Loop through the all the vertices and, based on a vector from the light 
    // to the vertex itself, calculate the correct shifted texture coordinates 
    // for emboss bump mapping.
    //

    for( int i = 0; i < NUM_VERTICES; ++i )
    {
        //
	    // Create a light vector in tangent space, which points from the 
        // light's position to the current vertice's position.
	    //

        D3DXVECTOR3 vLightToVertex = vLightsPosition - D3DXVECTOR3( pv[i].x, pv[i].y, pv[i].z );
        D3DXVec3Normalize( &vLightToVertex, &vLightToVertex );
        
        float r = D3DXVec3Dot( &vLightToVertex, &D3DXVECTOR3( pv[i].nx, pv[i].ny, pv[i].nz ) );

        if( r < 0.0f )
        {
            // Don't shift the coordinates when light is below the surface
            pv[i].tu2 = pv[i].tu;
            pv[i].tv2 = pv[i].tv;
        }
        else
        {
            // Shift coordinates, in tangent space, for the emboss effect.
            D3DXVECTOR2 vEmbossShift;
            vEmbossShift.x = D3DXVec3Dot( &vLightToVertex, &g_vTangents[i] );
            vEmbossShift.y = D3DXVec3Dot( &vLightToVertex, &g_vBiNormals[i] );
            D3DXVec2Normalize( &vEmbossShift, &vEmbossShift );
            pv[i].tu2 = pv[i].tu + (vEmbossShift.x / d3dsd.Width)  * g_fEmbossFactor;
            pv[i].tv2 = pv[i].tv - (vEmbossShift.y / d3dsd.Height) * g_fEmbossFactor;
        }
    }

    g_pVertexBuffer->Unlock();
}

//-----------------------------------------------------------------------------
// Name: renderQuadWithEmbossBumpMapping()
// Desc: Perform one-pass, emboss bump mapping
//
// Note: Both Texture Stages will use the same texture, becuase the height 
//       map needed for emboss bump mapping is being stored in the alpha 
//       channel of the  base texture.
//
//       This means that Texture Stage 0 will work only with the RGB 
//       components of the texture, and Texture Stage 1, which does the 
//       actual bump mapping, will use the texture's alpha component to 
//       create the bumps.
//-----------------------------------------------------------------------------
void renderQuadWithEmbossBumpMapping( void )
{
	//
	// Texture Stage 0:
	//

	g_pd3dDevice->SetTexture(0, g_baseTexture);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//
	// Texture Stage 1:
	//

	g_pd3dDevice->SetTexture(1, g_baseTexture);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE | D3DTA_COMPLEMENT);

	//
	// Texture Stage 2:
	//

	g_pd3dDevice->SetTexture(2, NULL);
	g_pd3dDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0);
	g_pd3dDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	g_pd3dDevice->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	g_pd3dDevice->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT | D3DTA_ALPHAREPLICATE);
	g_pd3dDevice->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	g_pd3dDevice->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	//
    // Render the quad...
	//

	g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
	g_pd3dDevice->SetVertexShader( Vertex::FVF_Flags );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	//
    // Reset render states...
	//

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	g_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	/*

  	//
	// One-pass "Fake" Embossed Bump Mapping (2 Texture Stages)
	//
	// Fake emboss bump mapping is generally used with hardware renderers 
	// which are limited to two multitexture layers. It requires that the base 
	// texture and diffuse color be artificially brightened to account for the 
	// missing factor of 2 in the final modulate operation (i.e. resulting 
	// image will be darker than normal). However this can produce clamping 
	// problems on bright colours.
	//

	//
	// Texture Stage 0: (just render base texture with diffuse)
	//

    g_pd3dDevice->SetTexture( 0, g_baseTexture );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	//
    // Texture Stage 1:
	//

    g_pd3dDevice->SetTexture( 1, g_baseTexture );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	//
    // Set up alpha blending:
	//

    
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

	//
    // Render the quad...
	//

    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( Vertex::FVF_Flags );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    //
    // Reset render states...
	//

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	//*/
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    if( g_bMoveLightAbout == true )
    {
		//
		// Spin the light around the quad...
		//
		
		static float fAngle = 0; 
		fAngle += 60 * g_fElpasedTime;

		// Wrap it around, if it gets too big
		while(fAngle > 360.0f) fAngle -= 360.0f;
		while(fAngle < 0.0f)   fAngle += 360.0f;

        float x = sinf( D3DXToRadian(fAngle) );
        float y = cosf( D3DXToRadian(fAngle) );

        g_pLight0.Position = D3DXVECTOR3( 2.0f * x, 
			                              2.0f * y, 
			                              0.0f );
    }

    //
    // Set up the model-view matrix to spin our quad via mouse input...
    //

    D3DXMATRIX matWorld;
	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    matWorld = matRot * matTrans;
	
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    //
    // If true, render the quad with emboss bump mapping...
    //

    if( g_bEmbossBumpMap )
    {
        // Shift or offset the texture coordinates used by Texture Stage 1
        shiftTextureCoordinates();

		// Render the quad without any emboss bump mapping... very boring!
		renderQuadWithEmbossBumpMapping();
    }
    else
    {
        // Render the quad without any emboss bump mapping...
        g_pd3dDevice->SetTexture( 0, g_baseTexture );

        g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
        g_pd3dDevice->SetVertexShader( Vertex::FVF_Flags );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    }

	//
	// Render a small sphere to mark the light's general position...
	//
	// Note: The small sphere is simply being used as a visual cue and 
	//       doesn't represent the light's true position on the z axis.
	//       The actual light is much further away, so the embossing effect 
	//       will look sharper.
	//

	g_pd3dDevice->SetTexture( 0, NULL );

	D3DXMatrixIdentity( &matWorld );
	// Use x and y, but move the sphere closer in so we can see it.
	D3DXMatrixTranslation( &matWorld, g_pLight0.Position.x, 
									  g_pLight0.Position.y, 
									  4.0f ); 

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pSphereMesh->DrawSubset(0);

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

