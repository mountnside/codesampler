//-----------------------------------------------------------------------------
//           Name: dx9_material.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use materials with lighting 
//                 to produce different surface effects.
//
//   Control Keys: Left Mouse Button - Spin the view
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd        = NULL;
LPDIRECT3D9       g_pD3D        = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice  = NULL;
LPD3DXMESH        g_pTeapotMesh = NULL;
D3DLIGHT9         g_pLight0;

D3DMATERIAL9 g_redClayMtrl;
D3DMATERIAL9 g_greenPlasticMtrl;
D3DMATERIAL9 g_silverMetalMtrl;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;    // Position of vertex in 3D space
    float nx, ny, nz; // Normal for lighting calculations
    DWORD diffuse;    // Diffuse color of vertex

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE
	};
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);

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
                             "Direct3D (DX9) - Material",
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
// Name: init()
// Desc: 
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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
						  D3DDEVTYPE_HAL,
						  g_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						  &d3dpp, &g_pd3dDevice );

    D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

    //-------------------------------------------------------------------------
    // Setup material - Used to create a reddish clay teapot
    //-------------------------------------------------------------------------

    ZeroMemory( &g_redClayMtrl, sizeof(D3DMATERIAL9) );

    // A matte clay look is easy. All we really have to do is set the color to 
    // look like clay pottery.
    g_redClayMtrl.Diffuse.r = 1.0f;
    g_redClayMtrl.Diffuse.g = 0.5f;
    g_redClayMtrl.Diffuse.b = 0.2f;
    g_redClayMtrl.Diffuse.a = 1.0f;

    //-------------------------------------------------------------------------
    // Setup material - Used to create a green plastic teapot
    //-------------------------------------------------------------------------

    ZeroMemory( &g_greenPlasticMtrl, sizeof(D3DMATERIAL9) );

    // Set the material's main color to green.
    g_greenPlasticMtrl.Diffuse.r = 0.0f;
    g_greenPlasticMtrl.Diffuse.g = 1.0f;
    g_greenPlasticMtrl.Diffuse.b = 0.0f;
    g_greenPlasticMtrl.Diffuse.a = 1.0f;

    // Lets favor the ambient's green over the other colors. Why? I don't know.
    // It just looks better to me. Using materials is some what more artistic
    // than scientific, so just play around till you get what you want.
    g_greenPlasticMtrl.Ambient.r = 0.5f;
    g_greenPlasticMtrl.Ambient.g = 1.0f;
    g_greenPlasticMtrl.Ambient.b = 0.5f;
    g_greenPlasticMtrl.Ambient.a = 1.0f;

    // Plastic can be shiny, but we don't want it too shiny are it will look 
    // more like glass or metal. We'll have the material reflect back more 
    // green than red and blue so the highlights won't be pure white.
    g_greenPlasticMtrl.Specular.r = 0.5f;
    g_greenPlasticMtrl.Specular.g = 1.0f;
    g_greenPlasticMtrl.Specular.b = 0.5f;
    g_greenPlasticMtrl.Specular.a = 1.0f;

    // It seems backwards, but increasing the Power value reduces the 
    // highlight's size
    g_greenPlasticMtrl.Power = 40.0f;

    //-------------------------------------------------------------------------
    // Setup material - Used to create a silver metallic teapot
    //-------------------------------------------------------------------------

    ZeroMemory( &g_silverMetalMtrl, sizeof(D3DMATERIAL9) );

    // Set the material's main color to a silver-like gray color.
    g_silverMetalMtrl.Diffuse.r = 0.5f;
    g_silverMetalMtrl.Diffuse.g = 0.5f;
    g_silverMetalMtrl.Diffuse.b = 0.5f;
    g_silverMetalMtrl.Diffuse.a = 1.0f;

    // A silver metal would be very shiny, so we'll reflect back all ambient.
    g_silverMetalMtrl.Ambient.r = 1.0f;
    g_silverMetalMtrl.Ambient.g = 1.0f;
    g_silverMetalMtrl.Ambient.b = 1.0f;
    g_silverMetalMtrl.Ambient.a = 1.0f;

    // We can make it seem extra shiny by having it actually emit some light 
	// of its own... but not too much are we'll wash the color out.
    g_silverMetalMtrl.Emissive.r = 0.1f;
    g_silverMetalMtrl.Emissive.g = 0.1f;
    g_silverMetalMtrl.Emissive.b = 0.1f;
    g_silverMetalMtrl.Emissive.a = 1.0f;

    // Polished silver can reflect back pure white highlights, so set the 
    // specular to pure white.
    g_silverMetalMtrl.Specular.r = 1.0f;
    g_silverMetalMtrl.Specular.g = 1.0f;
    g_silverMetalMtrl.Specular.b = 1.0f;
    g_silverMetalMtrl.Specular.a = 1.0f;

    // Set the Power value to a small number to make the highlight's size bigger.
    g_silverMetalMtrl.Power = 5.0f;

	//
    // If you're dealing with a model that provides its own per-vertex colors,
    // you'll probably need to disable the usage of vertex colors by Direct3D's 
    // lighting calculations to get the right effect. This can be accomplished 
	// by setting D3DRS_COLORVERTEX to FALSE like so...
	//
	// Comment out this line to use the teapot's real color, which is blue.
	//

    g_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );

    // Another way to ignore per-vertex colors is to simply force all lighting 
    // calculations to use your material settings explicitly like so...

    //g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
    //g_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
    //g_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
    //g_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );

    //
    // Setup a simple directional light and some ambient...
    //

	g_pLight0.Type = D3DLIGHT_DIRECTIONAL;
	g_pLight0.Direction = D3DXVECTOR3( 1.0f, 0.0f, 1.0f );

    g_pLight0.Diffuse.r = 1.0f;
    g_pLight0.Diffuse.g = 1.0f;
    g_pLight0.Diffuse.b = 1.0f;
	g_pLight0.Diffuse.a = 1.0f;

	g_pLight0.Specular.r = 1.0f;
	g_pLight0.Specular.g = 1.0f;
	g_pLight0.Specular.b = 1.0f;
	g_pLight0.Specular.a = 1.0f;

    g_pd3dDevice->SetLight( 0, &g_pLight0 );
    g_pd3dDevice->LightEnable( 0, TRUE );

    // Be careful when setting up ambient lighting. You can very easily wash 
    // out the material's specular highlights.

    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

    //
    // Load up the teapot mesh and then clone it so we can add per-vertex 
	// colors to it. I'm only doing this so I can demonstrate what happens if
	// we fail to ignore vertex colors when using materials. Failure to 
	// understand this causes many beginners a lot of headaches.
    //

    LPD3DXMESH pTempTeapotMesh = NULL;

    D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
                       NULL, NULL, NULL, NULL, &pTempTeapotMesh );

    LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;

    pTempTeapotMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pTeapotMesh );

    if( SUCCEEDED( g_pTeapotMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
    {
        int nNumVerts = g_pTeapotMesh->GetNumVertices();
        Vertex *pVertices = NULL;

        pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
        {
			// Make each vertex blue
            for( int i = 0; i < nNumVerts; ++i )
                pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 );
        }
        pTempVertexBuffer->Unlock();

        pTempVertexBuffer->Release();
    }

    pTempTeapotMesh->Release();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pTeapotMesh != NULL )
        g_pTeapotMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

	D3DXMATRIX matView;
    D3DXMATRIX matWorld;
	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 6.5f );
    matView = matTrans;
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// We'll reuse this rotation for all three teapots
	D3DXMatrixRotationYawPitchRoll( &matRot,
									D3DXToRadian(g_fSpinX), 
									D3DXToRadian(g_fSpinY), 
									0.0f );

    //
    // Render the first teapot using a red clay material
    //

	D3DXMatrixTranslation( &matTrans, 0.0f, 1.2f, 0.0f );
	matWorld = matRot * matTrans;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->SetMaterial( &g_redClayMtrl );
    g_pTeapotMesh->DrawSubset(0);

    //
    // Render the second teapot using a green plastic material
    //

    D3DXMatrixTranslation( &matTrans, 1.7f, -0.8f, 0.0f );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->SetMaterial( &g_greenPlasticMtrl );
    g_pTeapotMesh->DrawSubset(0);

    //
    // Render the third teapot using a silver metallic material
    //

    D3DXMatrixTranslation( &matTrans, -1.7f, -0.8f, 0.0f );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->SetMaterial( &g_silverMetalMtrl );
    g_pTeapotMesh->DrawSubset(0);

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

