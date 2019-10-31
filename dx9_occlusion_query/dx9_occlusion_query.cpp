//-----------------------------------------------------------------------------
//           Name: dx9_occlusion_query.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use Direct3D's new occlusion 
//                 query feature.
//
//   Control Keys: Left Mouse Button - Spin the view
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd          = NULL;
LPDIRECT3D9       g_pD3D          = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice    = NULL;
LPD3DXFONT        g_pd3dxFont     = NULL;
LPD3DXMESH        g_pSphereMesh   = NULL;
LPD3DXMESH        g_pPlaneMesh    = NULL;
LPDIRECT3DQUERY9  g_pSphereQuery  = NULL;
LPDIRECT3DQUERY9  g_pPlaneQuery   = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
	float x, y, z; // Position of vertex in 3D space
	DWORD diffuse; // Diffuse color of vertex

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE
	};
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void createFont(void);
void init(void);
void renderScene_toInitDepthBuffer(void);
void renderScene_toQuery(void);
void render(void);
void shutDown(void);

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
                             "Direct3D (DX9) - Occlusion Query",
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
// Name: createFont()
// Desc: 
//-----------------------------------------------------------------------------
void createFont( void )
{
    //
    // To create a Windows friendly font using only a point size, an 
    // application must calculate the logical height of the font.
    // 
    // This is because functions like CreateFont() and CreateFontIndirect() 
    // only use logical units to specify height.
    //
    // Here's the formula to find the height in logical pixels:
    //
    //             -( point_size * LOGPIXELSY )
    //    height = ----------------------------
    //                          72
    //

    HRESULT hr;
    HDC hDC;
    //HFONT hFont;
    int nHeight;
    int nPointSize = 9;
    //char strFontName[] = "Arial";

    hDC = GetDC( NULL );

    nHeight = -( MulDiv( nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72 ) );

    ReleaseDC( NULL, hDC );


    // Create a font for statistics and help output
    hr = D3DXCreateFont( g_pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                         DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), 
                         &g_pd3dxFont );

    if( FAILED( hr ) )
        MessageBox(NULL,"Call to D3DXCreateFont failed!", "ERROR",MB_OK|MB_ICONEXCLAMATION);
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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

    createFont();

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	//
	// Create two meshes to represent our sphere and plane...
	//

	LPD3DXMESH pTempSphereMesh = NULL;
	LPD3DXMESH pTempPlaneMesh = NULL;
	LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;

	D3DXCreateSphere(g_pd3dDevice, 0.25f, 20, 20, &pTempSphereMesh, NULL);
	D3DXCreateBox(g_pd3dDevice, 0.435f, 0.435f, 0.435f, &pTempPlaneMesh, NULL);

	//
	// Clone the original sphere mesh and make it red...
	//

	pTempSphereMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pSphereMesh );

	if( SUCCEEDED( g_pSphereMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int nNumVerts = g_pSphereMesh->GetNumVertices();
		Vertex *pVertices = NULL;

		pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
		{
			for( int i = 0; i < nNumVerts; ++i )
				pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 );
		}
		pTempVertexBuffer->Unlock();

		pTempVertexBuffer->Release();
	}

	//
	// Clone the original plane mesh and make it yellow...
	//

	pTempPlaneMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pPlaneMesh );

	if( SUCCEEDED( g_pPlaneMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int nNumVerts = g_pPlaneMesh->GetNumVertices();
		Vertex *pVertices = NULL;

		pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
		{
			for( int i = 0; i < nNumVerts; ++i )
				pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 );
		}
		pTempVertexBuffer->Unlock();

		pTempVertexBuffer->Release();
	}

	pTempSphereMesh->Release();
	pTempPlaneMesh->Release();

	//
	// Create our query objects...
	//

	// Check to see if device supports visibility query
	if( D3DERR_NOTAVAILABLE == g_pd3dDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, NULL ) )
	{
		MessageBox(NULL,"Couldn't create a Query object!","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	g_pd3dDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &g_pPlaneQuery );
	g_pd3dDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &g_pSphereQuery );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pSphereQuery != NULL )
		g_pSphereQuery->Release();

	if( g_pPlaneQuery != NULL )
		g_pPlaneQuery->Release();

	if( g_pSphereMesh != NULL )
		g_pSphereMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: renderScene_toInitDepthBuffer()
// Desc: 
//-----------------------------------------------------------------------------
void renderScene_toInitDepthBuffer( void )
{
	D3DXMATRIX matScale;
	D3DXMATRIX matTrans;
	D3DXMATRIX matWorld;

	//
	// Render the plane first...
	//

	D3DXMatrixScaling( &matScale, 1.0f, 1.0f, 0.05f );
	D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, -0.025f );
	matWorld = matScale * matTrans;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pPlaneMesh->DrawSubset(0);

	//
	// Render the sphere second...
	//

	D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 0.25f );
	matWorld = matTrans;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pSphereMesh->DrawSubset(0);
}

//-----------------------------------------------------------------------------
// Name: renderScene_toQuery()
// Desc: 
//-----------------------------------------------------------------------------
void renderScene_toQuery( void )
{
	D3DXMATRIX matScale;
	D3DXMATRIX matTrans;
	D3DXMATRIX matWorld;

	//
	// Render the plane first and wrap it with an occlusion query
	//

	g_pPlaneQuery->Issue( D3DISSUE_BEGIN );
	{
		D3DXMatrixScaling( &matScale, 1.0f, 1.0f, 0.05f );
		D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, -0.025f );
		matWorld = matScale * matTrans;
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
		g_pPlaneMesh->DrawSubset(0);
	}
	g_pPlaneQuery->Issue( D3DISSUE_END );

	//
	// Render the sphere second and wrap it with an occlusion query
	//

	g_pSphereQuery->Issue( D3DISSUE_BEGIN );
	{
		D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 0.25f );
		matWorld = matTrans;
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
		g_pSphereMesh->DrawSubset(0);
	}
	g_pSphereQuery->Issue( D3DISSUE_END );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.35f,0.53f,0.7f,1.0f), 1.0f, 0 );

	g_pd3dDevice->BeginScene();

	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;
	D3DXMATRIX matView;

	D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 1.5f );
	D3DXMatrixRotationYawPitchRoll( &matRot, 
									D3DXToRadian(g_fSpinX), 
									D3DXToRadian(g_fSpinY), 
									0.0f );

	matView = matRot * matTrans;
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	//
	// The first time we render the scene is to initialize the depth buffer. 
	// If we don't do this, an object, which is rendered first, may generate a 
	// pixel count which is greater than 0 even when that object is later 
	// occluded completely by another object, which is closer to the view point.
	//
	// You can actually skip this step if you know for certain that you'll
	// be rendering and querying your scene's objects in back-to-front order.
	//

	renderScene_toInitDepthBuffer();

	//
	// The second time is for getting accurate visible fragment counts
	//

	renderScene_toQuery();

	//
	// Now, we collect the fragment counts from our two 3D objects to see  
	// whether or not either of them would have contributed anything to the  
	// frame buffer if they were rendered.
	//

	DWORD dwPlaneData;
	DWORD dwSphereData;

	while( g_pPlaneQuery->GetData( (void*)&dwPlaneData,
			                        sizeof(DWORD),
			                        D3DGETDATA_FLUSH) == S_FALSE )
	{
		// GetData is asynchronous so, if possible, try to find some extra work 
		// to do here if the hardware is still rendering our scene.
	}

	while( g_pSphereQuery->GetData( (void*)&dwSphereData,
			                        sizeof(DWORD),
			                        D3DGETDATA_FLUSH) == S_FALSE )
	{
		// GetData is asynchronous so, if possible, try to find some extra work 
		// to do here if the hardware is still rendering our scene.
	}

    RECT destRect1;
    SetRect( &destRect1, 5, 5, 0, 0 );

	RECT destRect2;
	SetRect( &destRect2, 5, 20, 0, 0 );

	char planeString[50];
	char sphereString[50];

	sprintf( planeString, "Plane Fragments = %d ", dwPlaneData );
	sprintf( sphereString, "Sphere Fragments = %d ", dwSphereData );

    g_pd3dxFont->DrawText( NULL, planeString, -1, &destRect1, DT_NOCLIP, 
                           D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    g_pd3dxFont->DrawText( NULL, sphereString, -1, &destRect2, DT_NOCLIP, 
                           D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

	g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
