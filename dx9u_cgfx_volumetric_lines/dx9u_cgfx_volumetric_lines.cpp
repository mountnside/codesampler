//-----------------------------------------------------------------------------
//           Name: dx9u_cgfx_volumetric_lines.cpp
//         Author: Adel Amro (adel_w_amro@hotmail.com)
//  Last Modified: November 13, 2007
//    Description: This sample demonstrates a technique for efficiently drawing
//                 what is often called volume lines. These are quads oriented
//                 and textured in such a way to be rendered as though they were
//                 thick lines. This sample is based on the OpenGL sample from
//                 Nvidia titled CG Volume Lines. It should work on low end
//                 cards.
//
//   Control Keys: Click and drag to spin the bunch of lines.
//                 The + and - keys to control line thickness.
//                 Page Down (PgDn) to cycle through available textures.
//                 Arrow keys: Move the bunch of lines.
//-----------------------------------------------------------------------------


# define STRICT
# define WIN32_LEAN_AND_MEAN

# include <windows.h>
# include <d3dx9.h>


// Instruct the linker to use these libraries.
# pragma comment (lib, "d3d9.lib" )
# pragma comment (lib, "d3dx9.lib" )



# define ARRAY_SIZE( x ) ( (sizeof((x))/sizeof((x)[0])) )

inline FLOAT frand01() { return (FLOAT)rand()/RAND_MAX; }
inline FLOAT frand( float fMin, float fMax ) { return fMin + (fMax - fMin) * ((float)rand()/RAND_MAX); }
inline FLOAT frand() { return (float)rand()/RAND_MAX + rand(); }



// Globals


// Array of texture file names.
const TCHAR* g_aFileNames[] = {
		TEXT( "1d_INNER1.png" ),
		TEXT( "1d_INNER2.png" ),
		TEXT( "1d_DEBUG.png" ),
		TEXT( "1d_DEBUG2.png" ),
		TEXT( "1d_glow1.png" )
};


HWND						g_hWnd = NULL;
LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9			g_pD3DDevice = NULL;
LPDIRECT3DTEXTURE9			g_pTexture = NULL;
LPD3DXEFFECT				g_pEffect = NULL;
D3DXVECTOR3					g_aLines[ 16 ];


INT				g_iCurrentTexture = 0;
FLOAT			g_fThickness = 1.f;


FLOAT g_fSpinX = 0.f;
FLOAT g_fSpinY = 0.f;
FLOAT g_fTX = 0.f;
FLOAT g_fTY = 0.f;
FLOAT g_fTZ = 14.f;



D3DXMATRIX g_mProjection;



// The vertex structure we'll be using for line drawing. Each line is defined as two vertices,
// and the vertex shader will create a quad from these two vertices. However, since the vertex
// shader can only process one vertex at a time, we need to store information in each vertex
// about the other vertex (the other end of the line).
struct TVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 otherPos;
	D3DXVECTOR4 texOffset;
	D3DXVECTOR3 thickness;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL |
		D3DFVF_TEX2 | // D3DFVF_TEX2 specifies we have two sets of texture coordinates.
		D3DFVF_TEXCOORDSIZE4(0) | // This specifies that the first (0th) tex coord set has size 4 floats.
		D3DFVF_TEXCOORDSIZE3(1); // Specifies that second tex coord set has size 2 floats.
};



// Prototypes

INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, INT );
LRESULT CALLBACK WindowProc( HWND, UINT, WPARAM, LPARAM );
HRESULT Init();
VOID ShutDown();
VOID Render();




// Application's entry point.
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nShowCmd )
{
	WNDCLASSEX winClass;
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = TEXT("WC_DX9_VOLUMELINES");
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm	   = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	RECT rWindow = { 0, 0, 640, 480 };
	AdjustWindowRect( &rWindow, WS_OVERLAPPEDWINDOW, FALSE );

	g_hWnd = CreateWindowEx( NULL, winClass.lpszClassName,
                             TEXT("Direct3D (DX9) - Volume Lines"),
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							 0, 0, rWindow.right - rWindow.left,
							 rWindow.bottom - rWindow.top,
							 NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nShowCmd );
    UpdateWindow( g_hWnd );

	if( FAILED( Init() ) )
	{
		MessageBox( g_hWnd, TEXT( "Direct3D initialization failure!" ), TEXT("ERROR!"), MB_OK | MB_ICONSTOP );
		return 1;
	}

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    Render();
	}

	ShutDown();

	UnregisterClass( winClass.lpszClassName, winClass.hInstance );

	return (INT)uMsg.wParam;
}







// Initialize the application.
HRESULT Init()
{
	HRESULT hr;

	g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	if( !g_pD3D ) return E_FAIL;

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed					= TRUE;
    d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat			= d3ddm.Format;
    d3dpp.EnableAutoDepthStencil	= TRUE;
    d3dpp.AutoDepthStencilFormat	= D3DFMT_D16;
    d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_ONE; // No need to burn the CPU.
	d3dpp.hDeviceWindow				= g_hWnd;

    hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_HARDWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pD3DDevice );

	if( FAILED( hr ) )
		return hr;

	// Load the texture.
	hr = D3DXCreateTextureFromFile( g_pD3DDevice, g_aFileNames[0], &g_pTexture );
	if( FAILED( hr ) )
		return hr;

	// Load the effect from file.
	LPD3DXBUFFER pErrors = NULL;
	hr = D3DXCreateEffectFromFile( g_pD3DDevice, TEXT("VolumeLines.fx"), NULL, NULL,
		0, NULL, &g_pEffect, &pErrors );
	if( FAILED( hr ) )
	{
		if( pErrors )
		{
			MessageBoxA( g_hWnd, (LPCSTR)pErrors->GetBufferPointer(), "Effect error", MB_OK | MB_ICONSTOP );
			pErrors->Release();
		}
		return hr;
	}

	g_pEffect->SetTexture( "lineTexture", g_pTexture );
	//g_pEffect->SetTexture( "texture1", g_pTexture );


	// Set up some device states.
	D3DXMatrixPerspectiveFovLH( &g_mProjection, 45.f,
		(float)d3dpp.BackBufferWidth / (float)d3dpp.BackBufferHeight,
		0.1f, 100.f );

	g_pD3DDevice->SetTransform( D3DTS_PROJECTION, &g_mProjection );
	g_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );


	// Now create a random list of lines (pairs of vertices) that we will render
	srand( GetTickCount() );
	for( UINT i=0; i<ARRAY_SIZE(g_aLines); i++ )
	{
		g_aLines[i].x = frand(-1, 1) * 5.f;
		g_aLines[i].y = frand(-1, 1) * 5.f;
		g_aLines[i].z = frand(-1, 1) * 5.f;
	}

	return S_OK;
}






// Free all resources.
VOID ShutDown()
{
	if( g_pTexture ) g_pTexture->Release();
	if( g_pEffect ) g_pEffect->Release();
	if( g_pD3DDevice ) g_pD3DDevice->Release();
	if( g_pD3D ) g_pD3D->Release();
}





VOID RenderLine( D3DXVECTOR3& v0, D3DXVECTOR3& v1 )
{
	TVertex vrts[4];
	vrts[0].pos = v0;		vrts[0].otherPos = v1;
	vrts[1].pos = v1;		vrts[1].otherPos = v0;
	vrts[2].pos = v0;		vrts[2].otherPos = v1;
	vrts[3].pos = v1;		vrts[3].otherPos = v0;

	vrts[0].thickness = D3DXVECTOR3( -g_fThickness, 0.f, g_fThickness * 0.5f );
	vrts[1].thickness = D3DXVECTOR3(  g_fThickness, 0.f, g_fThickness * 0.5f );
	vrts[2].thickness = D3DXVECTOR3(  g_fThickness, 0.f, g_fThickness * 0.5f );
	vrts[3].thickness = D3DXVECTOR3( -g_fThickness, 0.f, g_fThickness * 0.5f );

	vrts[0].texOffset = D3DXVECTOR4( g_fThickness, g_fThickness, 0.f, 0.f );
	vrts[1].texOffset = D3DXVECTOR4( g_fThickness, g_fThickness, 0.25f, 0.f );
	vrts[2].texOffset = D3DXVECTOR4( g_fThickness, g_fThickness, 0.f, 0.25f );
	vrts[3].texOffset = D3DXVECTOR4( g_fThickness, g_fThickness, 0.25f, 0.25f );

	g_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vrts, sizeof( TVertex ) );
}






VOID Render()
{

	g_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,70), 1.f, 0 );
	if( SUCCEEDED( g_pD3DDevice->BeginScene() ) )
	{
		D3DXMATRIX mWorld;
		D3DXMATRIX mWorldProjection;
		D3DXMATRIX mTranslation;
		D3DXMATRIX mRotation;

		D3DXMatrixRotationYawPitchRoll( &mRotation, D3DXToRadian( g_fSpinX ),
			D3DXToRadian( g_fSpinY ), 0.f );
		D3DXMatrixTranslation( &mTranslation, g_fTX, g_fTY, g_fTZ );//0.f, 0.f, 14.f );
		mWorld = mRotation * mTranslation;
		mWorldProjection = mWorld * g_mProjection;

		// We will not be using a viewing transformation, so the view matrix will be identity.
		g_pEffect->SetMatrix( "mWV", &mWorld );
		g_pEffect->SetMatrix( "mWVP", &mWorldProjection );

		
		UINT passes = 0;
		if( SUCCEEDED( g_pEffect->Begin( &passes, 0 ) ) )
		{
			g_pEffect->BeginPass( 0 );
			g_pD3DDevice->SetFVF( TVertex::FVF );
			for( UINT i=0; i < ARRAY_SIZE( g_aLines ) / 2; i++ )
				RenderLine( g_aLines[i*2], g_aLines[i*2 + 1] );
			g_pEffect->EndPass();
			g_pEffect->End();
		}
		
		g_pD3DDevice->EndScene();
	}
	g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
}









// The message procedure for the main window.
LRESULT CALLBACK WindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	static POINT ptLastCursorPos;
	static POINT ptCurrentCursorPos;
	static BOOL bMousing = FALSE;

	switch( Msg )
	{
	case WM_CLOSE:
		PostQuitMessage( 0 );
		break;

	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			PostQuitMessage( 0 );
		else if( wParam == VK_NEXT )
		{
			// Cycle textures.
			g_iCurrentTexture = (g_iCurrentTexture+1)%ARRAY_SIZE(g_aFileNames);
			g_pTexture->Release();
			HRESULT hr = D3DXCreateTextureFromFile( g_pD3DDevice, g_aFileNames[ g_iCurrentTexture ],
				&g_pTexture );
			if( SUCCEEDED( hr ) )
			{
				g_pEffect->SetTexture( "lineTexture", g_pTexture );
				//g_pEffect->SetTexture( "texture1", g_pTexture );
			}
			else
			{
				MessageBox( g_hWnd, TEXT( "Couldn't open texture file!" ), TEXT("ERROR"), MB_OK | MB_ICONSTOP );
				PostQuitMessage( 0 );
			}
		}
		else if( wParam == VK_ADD )
			g_fThickness += 0.03f;
		else if( wParam == VK_SUBTRACT )
			g_fThickness -= 0.03f;
		else if( wParam == VK_LEFT )
			g_fTX += 0.5f;
		else if( wParam == VK_RIGHT )
			g_fTX -= 0.5f;
		else if( wParam == VK_UP )
			g_fTZ -= 0.5f;
		else if( wParam == VK_DOWN )
			g_fTZ += 0.5f;
		break;



	case WM_LBUTTONDOWN:
		ptLastCursorPos.x = ptCurrentCursorPos.x = LOWORD(lParam);
		ptLastCursorPos.y = ptCurrentCursorPos.y = HIWORD(lParam);
		bMousing = TRUE;

		// Set capture so we know when the user releases the left button even
		// if it's not over the window.
		SetCapture( hWnd );
		break;



	case WM_MOUSELEAVE: // Stop mousing if mouse button is released or cursor leaves window.
	case WM_LBUTTONUP:
		bMousing = FALSE;
		SetCapture( NULL );
		break;


	case WM_MOUSEMOVE:
		ptCurrentCursorPos.x = LOWORD( lParam );
		ptCurrentCursorPos.y = HIWORD( lParam );
		if( bMousing )
		{
			g_fSpinX -= (ptCurrentCursorPos.x - ptLastCursorPos.x) / 3;
			g_fSpinY -= (ptCurrentCursorPos.y - ptLastCursorPos.y) / 3;
		}
		ptLastCursorPos = ptCurrentCursorPos;
		break;
	}
	return DefWindowProc( hWnd, Msg, wParam, lParam );
}
