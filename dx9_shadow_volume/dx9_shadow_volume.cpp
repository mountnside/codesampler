//-----------------------------------------------------------------------------
//           Name: dx9_shadow_volume.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create shadows using a 
//                 shadow volume with Direct3D
//
//   Control Keys: Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//
//                 Right Mouse - Spin Teapot
//                 F1          - Toggle shadow volume geometry
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"
#include "shadow_volume.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd       = NULL;
LPDIRECT3D9       g_pD3D       = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

D3DLIGHT9 g_light0;

LPDIRECT3DVERTEXBUFFER9 g_pFloorVB  = NULL;
LPDIRECT3DTEXTURE9      g_pFloorTex = NULL;
D3DMATERIAL9            g_pFloorMtrl;

LPD3DXMESH          g_pTeapotMesh      = NULL;
D3DMATERIAL9       *g_pTeapotMtrls     = NULL;
LPDIRECT3DTEXTURE9 *g_pTeapotTextures  = NULL;
unsigned long       g_dwTeapotNumMtrls = 0L;

D3DXMATRIXA16            m_matTeapot;
LPDIRECT3DVERTEXBUFFER9  m_pBigSquareVB;
ShadowVolume            *m_pShadowVolume;

bool g_bMakeShadowVolumeVisible   = false;
bool g_bTwoSidedStencilsAvailable = false;

POINT  g_ptLastMousePosit_L;
POINT  g_ptCurrentMousePosit_L;
bool   g_bMousing_L = false;
POINT  g_ptLastMousePosit_R;
POINT  g_ptCurrentMousePosit_R;
bool   g_bMousing_R = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

D3DXVECTOR3	g_vEye(0.0f, 0.0f, -10.0f); // Eye Position
D3DXVECTOR3	g_vLook(0.0f, 0.0f, 1.0f);  // Look Vector
D3DXVECTOR3	g_vUp(0.0f, 1.0f, 0.0f);    // Up Vector
D3DXVECTOR3	g_vRight(1.0f, 0.0f, 0.0f); // Right Vector

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct FloorVertex
{
    float x, y, z;
	float nx, ny, nz;
    float tu, tv;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
	};
};

FloorVertex g_quadVertices[] =
{
	{-1.0f, 0.0f,-1.0f,  0.0f,1.0f,0.0f,  0.0f, 1.0f },
	{-1.0f, 0.0f, 1.0f,  0.0f,1.0f,0.0f,  0.0f, 0.0f },
	{ 1.0f, 0.0f,-1.0f,  0.0f,1.0f,0.0f,  1.0f, 1.0f },
	{ 1.0f, 0.0f, 1.0f,  0.0f,1.0f,0.0f,  1.0f, 0.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void initGeometryAndLighting(void);
void shutDown(void);
void render(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void renderShadowToStencilBuffer(void);
void renderShadowToScene(void);

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
                             "Direct3D (DX9) - Shadow Volume",
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
	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;

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
					g_bMakeShadowVolumeVisible = !g_bMakeShadowVolumeVisible;
					break;
			}
		}
        break;

        case WM_LBUTTONDOWN:
		{
			g_bMousing_L = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			g_bMousing_L = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			g_bMousing_R = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			g_bMousing_R = false;
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
// Name: getRealTimeUserInput()
// Desc: 
//-----------------------------------------------------------------------------
void getRealTimeUserInput( void )
{
	//
	// Get mouse input...
	//

	POINT mousePosit;
	GetCursorPos( &mousePosit );
	ScreenToClient( g_hWnd, &mousePosit );

    g_ptCurrentMousePosit_L.x = mousePosit.x;
    g_ptCurrentMousePosit_L.y = mousePosit.y;

    g_ptCurrentMousePosit_R.x = mousePosit.x;
    g_ptCurrentMousePosit_R.y = mousePosit.y;

	D3DXMATRIX matRotation;

    if( g_bMousing_L )
    {
		int nXDiff = (g_ptCurrentMousePosit_L.x - g_ptLastMousePosit_L.x);
        int nYDiff = (g_ptCurrentMousePosit_L.y - g_ptLastMousePosit_L.y);
        
        if( nYDiff != 0 )
		{
			D3DXMatrixRotationAxis( &matRotation, &g_vRight, D3DXToRadian((float)nYDiff / 3.0f));
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &matRotation );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &matRotation );
		}

		if( nXDiff != 0 )
		{
			D3DXMatrixRotationAxis( &matRotation, &D3DXVECTOR3(0,1,0), D3DXToRadian((float)nXDiff / 3.0f) );
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &matRotation );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &matRotation );
		}
    }

    if( g_bMousing_R )
	{
		g_fSpinX -= (g_ptCurrentMousePosit_R.x - g_ptLastMousePosit_R.x);
		g_fSpinY -= (g_ptCurrentMousePosit_R.y - g_ptLastMousePosit_R.y);
	}

    g_ptLastMousePosit_L.x = g_ptCurrentMousePosit_L.x;
    g_ptLastMousePosit_L.y = g_ptCurrentMousePosit_L.y;

    g_ptLastMousePosit_R.x = g_ptCurrentMousePosit_R.x;
    g_ptLastMousePosit_R.y = g_ptCurrentMousePosit_R.y;

	//
	// Get keyboard input...
	//

	unsigned char keys[256];
	GetKeyboardState( keys );

    D3DXVECTOR3 tmpLook  = g_vLook;
	D3DXVECTOR3 tmpRight = g_vRight;

	// Up Arrow Key - View moves forward
	if( keys[VK_UP] & 0x80 )
		g_vEye -= tmpLook*-g_fMoveSpeed*g_fElpasedTime;

	// Down Arrow Key - View moves backward
	if( keys[VK_DOWN] & 0x80 )
		g_vEye += (tmpLook*-g_fMoveSpeed)*g_fElpasedTime;

	// Left Arrow Key - View side-steps or strafes to the left
	if( keys[VK_LEFT] & 0x80 )
		g_vEye -= (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

	// Right Arrow Key - View side-steps or strafes to the right
	if( keys[VK_RIGHT] & 0x80 )
		g_vEye += (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

	// Home Key - View elevates up
	if( keys[VK_HOME] & 0x80 )
		g_vEye.y += g_fMoveSpeed*g_fElpasedTime; 

	// End Key - View elevates down
	if( keys[VK_END] & 0x80 )
		g_vEye.y -= g_fMoveSpeed*g_fElpasedTime;
}

//-----------------------------------------------------------------------------
// Name : updateViewMatrix()
// Desc : Builds a view matrix suitable for Direct3D.
//
// Here's what the final matrix should look like:
//
//  |   rx     ux     lx    0 |
//  |   ry     uy     ly    0 |
//  |   rz     uz     lz    0 |
//  | -(r.e) -(u.e) -(l.e)  1 |
//
// Where r = Right vector
//       u = Up vector
//       l = Look vector
//       e = Eye position in world space
//       . = Dot-product operation
//
//-----------------------------------------------------------------------------
void updateViewMatrix( void )
{
	D3DXMATRIX view;
	D3DXMatrixIdentity( &view );

	D3DXVec3Normalize( &g_vLook, &g_vLook );
	D3DXVec3Cross( &g_vRight, &g_vUp, &g_vLook );
	D3DXVec3Normalize( &g_vRight, &g_vRight );
	D3DXVec3Cross( &g_vUp, &g_vLook, &g_vRight );
	D3DXVec3Normalize( &g_vUp, &g_vUp );

	view._11 = g_vRight.x;
    view._12 = g_vUp.x;
    view._13 = g_vLook.x;
	view._14 = 0.0f;

	view._21 = g_vRight.y;
    view._22 = g_vUp.y;
    view._23 = g_vLook.y;
	view._24 = 0.0f;

	view._31 = g_vRight.z;
    view._32 = g_vUp.z;
    view._33 = g_vLook.z;
	view._34 = 0.0f;

	view._41 = -D3DXVec3Dot( &g_vEye, &g_vRight );
	view._42 = -D3DXVec3Dot( &g_vEye, &g_vUp );
	view._43 = -D3DXVec3Dot( &g_vEye, &g_vLook );
	view._44 =  1.0f;

	g_pd3dDevice->SetTransform( D3DTS_VIEW, &view ); 
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	//
	// Shadow stenciling will be faster if D3DSTENCILCAPS_TWOSIDED is 
	// supported.
	//

	D3DCAPS9 d3dCaps;
	g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps );
	if( ( d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED ) != 0 )
		g_bTwoSidedStencilsAvailable = true;

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags                  = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          //D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
                         &d3dpp, &g_pd3dDevice );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 500.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	initGeometryAndLighting();
}

//-----------------------------------------------------------------------------
// Name: initGeometryAndLighting()
// Desc: 
//-----------------------------------------------------------------------------
void initGeometryAndLighting( void )
{
	//
	// Load up our teapot from a .x file...
	//
	
	LPD3DXBUFFER pD3DXMtrlBuffer;

    D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, NULL, 
		               &pD3DXMtrlBuffer, NULL, &g_dwTeapotNumMtrls, &g_pTeapotMesh );

    // We need to extract the material properties and texture names 
    // from the pD3DXMtrlBuffer
    D3DXMATERIAL *d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_pTeapotMtrls = new D3DMATERIAL9[g_dwTeapotNumMtrls];
    g_pTeapotTextures = new LPDIRECT3DTEXTURE9[g_dwTeapotNumMtrls];

    for( unsigned long i = 0; i < g_dwTeapotNumMtrls; ++i )
    {
        // Copy the material over...
        g_pTeapotMtrls[i] = d3dxMaterials[i].MatD3D;

        // Set the ambient color for the material (D3DX does not do this)
        g_pTeapotMtrls[i].Ambient = g_pTeapotMtrls[i].Diffuse;
     
		// Create the texture...
		g_pTeapotTextures[i] = NULL;
        D3DXCreateTextureFromFile( g_pd3dDevice, d3dxMaterials[i].pTextureFilename, 
                                   &g_pTeapotTextures[i] );
    }

    pD3DXMtrlBuffer->Release();

	//
	// Cereate a single floor tile and its texture...
	//

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(FloorVertex),0, FloorVertex::FVF_Flags,
                                      D3DPOOL_DEFAULT, &g_pFloorVB, NULL );
	void *pVertices = NULL;

    g_pFloorVB->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pFloorVB->Unlock();

	D3DXCreateTextureFromFile( g_pd3dDevice, "floor_tile.bmp", &g_pFloorTex );

    // Set up a material for the floor quads
	ZeroMemory( &g_pFloorMtrl, sizeof(D3DMATERIAL9) );
    g_pFloorMtrl.Diffuse.r = 1.0f;
    g_pFloorMtrl.Diffuse.g = 1.0f;
    g_pFloorMtrl.Diffuse.b = 1.0f;
    g_pFloorMtrl.Diffuse.a = 1.0f;
    g_pFloorMtrl.Ambient.r = 1.0f;
    g_pFloorMtrl.Ambient.g = 1.0f;
    g_pFloorMtrl.Ambient.b = 1.0f;
    g_pFloorMtrl.Ambient.a = 1.0f;

	//
	// Set up a point light source...
	//

	ZeroMemory( &g_light0, sizeof(D3DLIGHT9) );
	g_light0.Type         = D3DLIGHT_POINT;
	g_light0.Position     = D3DXVECTOR3( 2.0f, 2.0f, 0.0f ); // World-space position
	g_light0.Diffuse.r    = 1.0f;
    g_light0.Diffuse.g    = 1.0f;
    g_light0.Diffuse.b    = 1.0f;
    g_light0.Range        = 100.0f;
	g_light0.Attenuation0 = 1.0f;
    g_light0.Attenuation1 = 0.0f;

    g_pd3dDevice->SetLight( 0, &g_light0 );
	g_pd3dDevice->LightEnable( 0, TRUE );

	//
	// Create a big square for rendering the stencil buffer contents...
	//

    g_pd3dDevice->CreateVertexBuffer( 4*sizeof(ShadowVertex),
                                      D3DUSAGE_WRITEONLY, ShadowVertex::FVF_Flags,
                                      D3DPOOL_MANAGED, &m_pBigSquareVB, NULL );

	// Get the width & height of the back-buffer.
    LPDIRECT3DSURFACE9 pBackBuffer = NULL;
	D3DSURFACE_DESC d3dsd;
    g_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &d3dsd );
    pBackBuffer->Release();
	float sx = (float)d3dsd.Width;
	float sy = (float)d3dsd.Height;

	// Set the size of the big square shadow
    ShadowVertex *v;

    m_pBigSquareVB->Lock( 0, 0, (void**)&v, 0 );
	{
		v[0].p = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
		v[1].p = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
		v[2].p = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
		v[3].p = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
		v[0].color = 0x7f000000;
		v[1].color = 0x7f000000;
		v[2].color = 0x7f000000;
		v[3].color = 0x7f000000;
	}
    m_pBigSquareVB->Unlock();

	//
	// Finally, construct a shadow volume object...
	//

    m_pShadowVolume = new ShadowVolume();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pTeapotMtrls != NULL )
        delete[] g_pTeapotMtrls;

    if( g_pTeapotTextures != NULL )
    {
        for( unsigned long i = 0; i < g_dwTeapotNumMtrls; ++i )
		{
			if( g_pTeapotTextures[i] != NULL )
				g_pTeapotTextures[i]->Release();
		}
	
        delete[] g_pTeapotTextures;
    }

    if( g_pFloorVB != NULL )
        g_pFloorVB->Release(); 

	if( g_pTeapotMesh != NULL )
        g_pTeapotMesh->Release();
	
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: renderShadowToStencilBuffer()
// Desc:
//-----------------------------------------------------------------------------
void renderShadowToStencilBuffer( void )
{
    // Disable z-buffer writes (note: z-testing still occurs), and enable the
    // stencil-buffer
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );

    // Dont bother with interpolating color
    g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

    // Set up stencil compare fuction, reference value, and masks.
    // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
    // Note: since we set up the stencil-test to always pass, the STENCILFAIL
    // renderstate is really not needed.
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );

    // If z-test passes, inc/decrement stencil buffer value
    g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );

    // Make sure that no pixels get drawn to the frame buffer
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	if( g_bTwoSidedStencilsAvailable == true )
    {
        // With 2-sided stencil, we can avoid rendering twice:
        g_pd3dDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );
        g_pd3dDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP );
        g_pd3dDevice->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
        g_pd3dDevice->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR );

        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE );

        // Draw both sides of shadow volume in stencil/z only
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTeapot );
        m_pShadowVolume->render( g_pd3dDevice );

        g_pd3dDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
    }
    else
    {
        // Draw front-side of shadow volume in stencil/z only
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTeapot );
        m_pShadowVolume->render( g_pd3dDevice );

        // Now reverse cull order so back sides of shadow volume are written.
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

        // Decrement stencil buffer value
        g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );

        // Draw back-side of shadow volume in stencil/z only
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTeapot );
        m_pShadowVolume->render( g_pd3dDevice );
    }

    // Restore render states
    g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}

//-----------------------------------------------------------------------------
// Name: renderShadowToScene()
// Desc: Draws a big gray polygon over scene according to the mask in the
//       stencil buffer. (Any pixel with stencil==1 is in the shadow.)
//-----------------------------------------------------------------------------
void renderShadowToScene( void )
{
    // Set render states
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

    // Only write where stencil value >= 1 (count indicates # of shadows that
    // overlap that pixel)
    g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );

    // Draw a big, gray square
    g_pd3dDevice->SetFVF( ShadowVertex::FVF_Flags );
    g_pd3dDevice->SetStreamSource( 0, m_pBigSquareVB, 0, sizeof(ShadowVertex) );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Restore render states
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	//
	// The first thing we need to do is setup the matrix that's going to be 
	// used by the teapot when rendered. The shadow volume needs to know this
	// in advance when creating the shadow volume.
	//

	D3DXMatrixIdentity( &m_matTeapot );
	D3DXMatrixRotationYawPitchRoll( &m_matTeapot,
									D3DXToRadian(g_fSpinX),
									D3DXToRadian(g_fSpinY),
									0.0f );

	//
	// Next, transform the light vector from world-space to object-space, so 
	// the teapot and light will both be in the same frame of reference.
	//
	
	D3DXVECTOR3 vLightInWorldSpace( g_light0.Position.x, g_light0.Position.y, g_light0.Position.z );
    D3DXVECTOR3 vLightInObjectSpace;
    D3DXMATRIXA16 matInverse;
    D3DXMatrixInverse( &matInverse, NULL, &m_matTeapot );
	D3DXVec3TransformNormal( &vLightInObjectSpace, &vLightInWorldSpace, &matInverse );

	//
	// Using the light's object-space position, build the shadow volume 
	// from the teapot's DX9 mesh.
	//

    m_pShadowVolume->reset();
	m_pShadowVolume->buildShadowVolume( g_pTeapotMesh, vLightInObjectSpace );

	//
	// Now, we're ready to render the scene...
	//

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
                         D3DCOLOR_COLORVALUE(0.35f, 0.53f, 0.7, 1.0f), 1.0f, 0 );

	getRealTimeUserInput();
	updateViewMatrix();

    g_pd3dDevice->BeginScene();

	//
	// Rendering the teapot mesh...
	//

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTeapot );

    for( unsigned long n = 0; n < g_dwTeapotNumMtrls; ++n )
    {
        g_pd3dDevice->SetMaterial( &g_pTeapotMtrls[n] );
        g_pd3dDevice->SetTexture( 0, g_pTeapotTextures[n] );
        g_pTeapotMesh->DrawSubset( n );
    }

	//
	// Draw the floor by rendering a 25 x 25 grouping of textured quads.
	//

	g_pd3dDevice->SetTexture( 0, g_pFloorTex );
	g_pd3dDevice->SetMaterial( &g_pFloorMtrl );
    g_pd3dDevice->SetStreamSource( 0, g_pFloorVB, 0, sizeof(FloorVertex) );
    g_pd3dDevice->SetFVF( FloorVertex::FVF_Flags );

    D3DXMATRIX matFloor;
	float x = 0.0f;
	float z = 0.0f;

	for( int i = 0; i < 25; ++i )
    {
		for( int j = 0; j < 25; ++j )
		{
			D3DXMatrixTranslation( &matFloor, x - 25.0f, -3.0f, z - 25.0f );
			g_pd3dDevice->SetTransform( D3DTS_WORLD, &matFloor );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);

			x += 2.0f;
		}
		x  =  0.0f;
		z += 2.0f;
	}

	//
	// If the user selected this option, render the shadow volume geometry...
	//

	if( g_bMakeShadowVolumeVisible == true )
	{
		// Render shadow volume in wire-frame mode
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTeapot );
		m_pShadowVolume->render( g_pd3dDevice );
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	//
    // Render the shadow volume into the stencil buffer, then render it 
    // to the scene.
	//

    renderShadowToStencilBuffer();
    renderShadowToScene();

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

