//-----------------------------------------------------------------------------
//           Name: dx8_first_person_shooter.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Source code for a First Person Shooter style view using 
//                 DirectInput.
//
//       Controls: Up Arrow    = Moves view forward
//                 Down Arrow  = Moves view backwards
//                 Left Arrow  = View strafes to the left
//                 Right Arrow = View strafes to the right
//                 Home Key    = View gains altitude
//                 End Key     = View loses altitude
//                 Left Mouse  = Perform mouse-looking
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include <windows.h>
#include <mmsystem.h>
#include <dinput.h>
#include <d3dx8.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// MACROS / DEFINES
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80)

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECT3D8             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER8 g_pvbFloor   = NULL; // Buffer to hold floor tile vertices for rendering purposes
LPDIRECT3DTEXTURE8      g_ptexFloor  = NULL; // Floor texture

LPDIRECTINPUT8          g_lpdi       = NULL; // DirectInput object
LPDIRECTINPUTDEVICE8    g_pKeyboard  = NULL; // DirectInput device for keyboard
LPDIRECTINPUTDEVICE8    g_pMouse     = NULL; // DirectInput device for mouse

// Our custom FVF, which describes our custom vertex structure below
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

struct Vertex
{
    float x, y, z;
    float tu, tv;
};

Vertex g_quadVertices[] =
{
	{-5.0f, 0.0f,-5.0f,  0.0f, 1.0f },
	{-5.0f, 0.0f, 5.0f,  0.0f, 0.0f },
	{ 5.0f, 0.0f,-5.0f,  1.0f, 1.0f },
	{ 5.0f, 0.0f, 5.0f,  1.0f, 0.0f }
};

float  g_fMoveSpeed = 10.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

D3DXVECTOR3	g_vEye(0.0f, 20.0f, 0.0f);  // Camera Position
D3DXVECTOR3	g_vLook(0.0f, 0.0f, 1.0f);  // Camera Look Vector
D3DXVECTOR3	g_vUp(0.0f, 1.0f, 0.0f);    // Camera Up Vector
D3DXVECTOR3	g_vRight(1.0f, 0.0f, 0.0f); // Camera Right Vector

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
HRESULT initD3D(HWND hWnd);
void shutDownD3D(void);
void initDInput(HWND hWnd);
void shutDownDInput(void);
void render(HWND hWnd);
void updateViewMatrix(void);
HRESULT	readUserInput(void);

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
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
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	HWND       hWnd;
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName	= "MY_WINDOWS_CLASS";
	winClass.cbSize         = sizeof(WNDCLASSEX);
	winClass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc	= WindowProc;
	winClass.hInstance		= hInstance;
	winClass.hIcon	        = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm	    = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName	= NULL;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						   "Direct3D (DX8) - First-Person-Shooter style view control",
                           WS_OVERLAPPEDWINDOW, 0,0, 640,480, NULL, NULL, 
                           hInstance, NULL );

	if( hWnd == NULL )
		return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

	initD3D( hWnd );
	initDInput( hWnd );

	g_dLastTime	= timeGetTime();
	
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

			readUserInput();
		    render( hWnd );
		}
	}

	shutDownDInput();
	shutDownD3D();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: initD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT initD3D( HWND hWnd )
{
    g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_FLIP;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

	// Create a texture and mipmap it...
	D3DXCreateTextureFromFile( g_pd3dDevice, "dx.bmp", &g_ptexFloor );
	
	g_pd3dDevice->CreateTexture( 256, 256, 0, 0, D3DFMT_UNKNOWN,
								 D3DPOOL_MANAGED, &g_ptexFloor );

	g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

	// Create a single floor tile as a simple quad ready for texturing...
    g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex),0, D3DFVF_CUSTOMVERTEX,
                                      D3DPOOL_DEFAULT, &g_pvbFloor );
	void *pVertices;

    g_pvbFloor->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pvbFloor->Unlock();

    // Set up some general Direct3D settings...
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );// Turn off lighting
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );  // Turn on the z-buffer

	// Set up the projection
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 500.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDownD3D()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDownD3D( void )
{
	SAFE_RELEASE( g_pvbFloor );
	SAFE_RELEASE( g_ptexFloor );
	SAFE_RELEASE( g_pd3dDevice );
	SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
// Name: initDInput()
// Desc: Creates a keyboard and mouse device using DirectInput
//-----------------------------------------------------------------------------
void initDInput( HWND hWnd )
{
    shutDownDInput();

    DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                        IID_IDirectInput8, (VOID**)&g_lpdi, NULL );
    
    // Create a keyboard device...
    g_lpdi->CreateDevice( GUID_SysKeyboard, &g_pKeyboard, NULL );
    g_pKeyboard->SetDataFormat( &c_dfDIKeyboard );
    g_pKeyboard->SetCooperativeLevel( hWnd, DISCL_NOWINKEY | DISCL_NONEXCLUSIVE | 
		                              DISCL_FOREGROUND );
    g_pKeyboard->Acquire();

    // Create a mouse device...
    g_lpdi->CreateDevice( GUID_SysMouse, &g_pMouse, NULL );
    g_pMouse->SetDataFormat( &c_dfDIMouse2 );
    g_pMouse->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );

    g_pMouse->Acquire();
}

//-----------------------------------------------------------------------------
// Name: shutDownDInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
void shutDownDInput( void )
{
    if( g_pKeyboard ) 
        g_pKeyboard->Unacquire();

	if( g_pMouse )
		g_pMouse->Unacquire();
    
	SAFE_RELEASE( g_pMouse );
    SAFE_RELEASE( g_pKeyboard );
    SAFE_RELEASE( g_lpdi );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render the scene
//-----------------------------------------------------------------------------
void render( HWND hWnd )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                         D3DCOLOR_COLORVALUE( 0.350f, 0.530f, 0.701, 1.0f ), 
                         1.0f, 0 );

    g_pd3dDevice->BeginScene();

	updateViewMatrix();
	
	// Render the floor...
    g_pd3dDevice->SetTexture( 0, g_ptexFloor );
    g_pd3dDevice->SetStreamSource( 0, g_pvbFloor, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );

    D3DXMATRIX matFloor;
	float x = 0.0f;
	float z = 0.0f;

	for( int i = 0; i < 10; ++i )
    {
		for( int j = 0; j < 10; ++j )
		{
			D3DXMatrixTranslation( &matFloor, x, 0.0f, z );
			g_pd3dDevice->SetTransform( D3DTS_WORLD, &matFloor );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);

			x += 10.0f;
		}
		x  =  0.0f;
		z += 10.0f;
	}

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

}

//-----------------------------------------------------------------------------
// Name: readUserInput()
// Desc: Reads user input and modifies a set of globals that will later be 
//       used to build the camera's current view.
//-----------------------------------------------------------------------------
HRESULT readUserInput( void )
{
	DIMOUSESTATE2 dims;
	char buffer[256];
	HRESULT hr;

    ZeroMemory( &dims, sizeof(dims) );

	if( FAILED( hr = g_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims ) ) )
	{
		// If input is lost then acquire and keep trying
		hr = g_pMouse->Acquire();
		while( hr == DIERR_INPUTLOST )
			hr = g_pMouse->Acquire();
		return S_OK;
	}

	if( FAILED( hr = g_pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer) ) )
        return hr;

    D3DXMATRIX mtxRot;

    // If the left mouse button is down - do mouse looking
	if( dims.rgbButtons[0] & 0x80 )
	{
		if( dims.lY != 0 )
		{
			D3DXMatrixRotationAxis( &mtxRot, &g_vRight, D3DXToRadian((float)dims.lY / 3.0f));
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
		}

		if( dims.lX != 0 )
		{
			D3DXMatrixRotationAxis( &mtxRot, &D3DXVECTOR3(0,1,0), D3DXToRadian((float)dims.lX / 3.0f) );
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
		}
	}

	D3DXVECTOR3 tmpLook  = g_vLook;
	D3DXVECTOR3 tmpRight = g_vRight;

    // View moves forward
	if( KEYDOWN(buffer, DIK_UP) )
		g_vEye += (g_fMoveSpeed*tmpLook)*g_fElpasedTime;

    // View moves backward
	if( KEYDOWN(buffer, DIK_DOWN) )
		g_vEye -= (g_fMoveSpeed*tmpLook)*g_fElpasedTime;

	// View side-steps or strafes to the left
	if( KEYDOWN(buffer, DIK_LEFT) )
		g_vEye -= (g_fMoveSpeed*tmpRight)*g_fElpasedTime;

	// View side-steps or strafes to the right
	if( KEYDOWN(buffer, DIK_RIGHT) )
		g_vEye += (g_fMoveSpeed*tmpRight)*g_fElpasedTime;

	// View elevates up
	if( KEYDOWN(buffer, DIK_HOME) )
		g_vEye.y += g_fMoveSpeed*g_fElpasedTime; 
	
    // View elevates down
	if( KEYDOWN(buffer, DIK_END) )
		g_vEye.y -= g_fMoveSpeed*g_fElpasedTime;

	return S_OK;
}
  
//-----------------------------------------------------------------------------
// Name : UpdateViewPos ()
// Desc : Updates our camera position and matrices.
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
