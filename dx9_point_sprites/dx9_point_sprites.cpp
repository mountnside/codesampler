//-----------------------------------------------------------------------------
//           Name: dx9_point_sprites.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use hardware accelerated 
//                 point sprites with Direct3D.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    D3DXVECTOR3 posit;
    D3DCOLOR    color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ|D3DFVF_DIFFUSE
	};
};

struct Particle
{
    D3DXVECTOR3 m_vCurPos;
    D3DXVECTOR3 m_vCurVel;
	D3DCOLOR    m_vColor;
};

const int MAX_PARTICLES = 100;
Particle g_particles[MAX_PARTICLES];

//-----------------------------------------------------------------------------
// PROTOYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);

void initPointSprites(void);
void updatePointSprites(void);
void renderPointSprites(void);

float getRandomMinMax(float fMin, float fMax);
D3DXVECTOR3 getRandomVector(void);

// Helper function to stuff a FLOAT into a DWORD argument
inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

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
                             "Direct3D (DX9) - Point Sprites",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
    initPointSprites();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
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
// Name: InitDirect3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
void init( void )
{
	g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );
	
	D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3(0.0f, 0.0f,-10.0f), 
		                          &D3DXVECTOR3(0.0f, 0.0f, 0.0f), 
		                          &D3DXVECTOR3(0.0f, 1.0f, 0.0f ) );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 1.0f, 1.0f, 200.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,  FALSE );
}


//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pVertexBuffer != NULL )        
        g_pVertexBuffer->Release();

    if( g_pTexture != NULL )
        g_pTexture->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: getRandomMinMax()
// Desc: Gets a random number between min/max boundaries
//-----------------------------------------------------------------------------
float getRandomMinMax( float fMin, float fMax )
{
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

//-----------------------------------------------------------------------------
// Name: getRandomVector()
// Desc: Generates a random vector where X,Y, and Z components are between
//       -1.0 and 1.0
//-----------------------------------------------------------------------------
D3DXVECTOR3 getRandomVector( void )
{
	D3DXVECTOR3 vVector;

    // Pick a random Z between -1.0f and 1.0f.
    vVector.z = getRandomMinMax( -1.0f, 1.0f );
    
    // Get radius of this circle
    float radius = (float)sqrt(1 - vVector.z * vVector.z);
    
    // Pick a random point on a circle.
    float t = getRandomMinMax( -D3DX_PI, D3DX_PI );

    // Compute matching X and Y for our Z.
    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

	return vVector;
}

//-----------------------------------------------------------------------------
// Name: initPointSprites()
// Desc: 
//-----------------------------------------------------------------------------
void initPointSprites( void )
{
	//
	// Load up the point sprite's texture...
	//

	D3DXCreateTextureFromFile( g_pd3dDevice, "particle.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//
	// Create a vertex bufer which can be used with point sprites...
	//

    g_pd3dDevice->CreateVertexBuffer( 2048 * sizeof(Vertex), 
                                      D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
									  Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
									  &g_pVertexBuffer, NULL );

	//
    // If you want to know the max size that a point sprite can be set to, 
	// and whether or not you can change the size of point sprites in hardware 
	// by sending D3DFVF_PSIZE with the FVF, do this.
	//

	float fMaxPointSize = 0.0f;
	bool  bDeviceSupportsPSIZE = false;
	
    D3DCAPS9 d3dCaps;
    g_pd3dDevice->GetDeviceCaps( &d3dCaps );

    fMaxPointSize = d3dCaps.MaxPointSize;

    if( d3dCaps.FVFCaps & D3DFVFCAPS_PSIZE )
        bDeviceSupportsPSIZE = true;
    else
        bDeviceSupportsPSIZE = false;

	//
	// Initialize our particles so they'll start at the origin with some 
	// random direction and color.
	//

	for( int i = 0; i < MAX_PARTICLES; ++i )
    {
		g_particles[i].m_vCurPos = D3DXVECTOR3(0.0f,0.0f,0.0f);
		g_particles[i].m_vCurVel = getRandomVector() * getRandomMinMax( 0.5f, 5.0f );

		g_particles[i].m_vColor = D3DCOLOR_COLORVALUE( getRandomMinMax( 0.0f, 1.0f ), 
                                                       getRandomMinMax( 0.0f, 1.0f ), 
                                                       getRandomMinMax( 0.0f, 1.0f ), 
                                                       1.0f );
	}
}

//-----------------------------------------------------------------------------
// Name: updatePointSprites()
// Desc: 
//-----------------------------------------------------------------------------
void updatePointSprites( void )
{
	//
	// To repeat the sample automatically, keep track of the overall app time.
	//

	static double dStartAppTime = timeGetTime();
	float fElpasedAppTime = (float)((timeGetTime() - dStartAppTime) * 0.001);

	//
	// To move the particles via their velocity, keep track of how much time  
	// has elapsed since last frame update...
	//

	static double dLastFrameTime = timeGetTime();
	double dCurrenFrameTime = timeGetTime();
	double dElpasedFrameTime = (float)((dCurrenFrameTime - dLastFrameTime) * 0.001);
	dLastFrameTime = dCurrenFrameTime;

	//
	// After 5 seconds, repeat the sample by returning all the particles 
	// back to the origin.
	//

    if( fElpasedAppTime >= 5.0f )
	{
		for( int i = 0; i < MAX_PARTICLES; ++i )
			g_particles[i].m_vCurPos = D3DXVECTOR3(0.0f,0.0f,0.0f);

		dStartAppTime = timeGetTime();
	}
	
	//
	// Move each particle via its velocity and elapsed frame time.
	//
	
	for( int i = 0; i < MAX_PARTICLES; ++i )
		g_particles[i].m_vCurPos += g_particles[i].m_vCurVel * (float)dElpasedFrameTime;
}

//-----------------------------------------------------------------------------
// Name: renderPointSprites()
// Desc: 
//-----------------------------------------------------------------------------
void renderPointSprites( void )
{
	//
	// Setting D3DRS_ZWRITEENABLE to FALSE makes the Z-Buffer read-only, which 
	// helps remove graphical artifacts generated from  rendering a list of 
	// particles that haven't been sorted by distance to the eye.
	//
    // Setting D3DRS_ALPHABLENDENABLE to TRUE allows particles, which overlap, 
	// to alpha blend with each other correctly.
	//

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	//
	// Set up the render states for using point sprites...
	//

    g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );    // Turn on point sprites
    g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );    // Allow sprites to be scaled with distance
    g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE,     FtoDW(1.0) );  // Float value that specifies the size to use for point size computation in cases where point size is not specified for each vertex.
    g_pd3dDevice->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(1.0f) ); // Float value that specifies the minimum size of point primitives. Point primitives are clamped to this size during rendering. 
    g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.0f) ); // Default 1.0
    g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.0f) ); // Default 0.0
    g_pd3dDevice->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(1.0f) ); // Default 0.0

	//
	// Lock the vertex buffer, and set up our point sprites in accordance with 
	// our particles that we're keeping track of in our application.
	//

	Vertex *pPointVertices;

	g_pVertexBuffer->Lock( 0, MAX_PARTICLES * sizeof(Vertex),
		                   (void**)&pPointVertices, D3DLOCK_DISCARD );

	for( int i = 0; i < MAX_PARTICLES; ++i )
    {
        pPointVertices->posit = g_particles[i].m_vCurPos;
        pPointVertices->color = g_particles[i].m_vColor;
        pPointVertices++;
    }

    g_pVertexBuffer->Unlock();
	
	//
	// Render point sprites...
	//

    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );
	g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, MAX_PARTICLES );

	//
    // Reset render states...
	//
	
    g_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: render the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;
	D3DXMATRIX matWorld;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 10.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	updatePointSprites();

    g_pd3dDevice->BeginScene();

	g_pd3dDevice->SetTexture( 0, g_pTexture );

	renderPointSprites();

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
