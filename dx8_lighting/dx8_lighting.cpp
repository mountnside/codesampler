//-----------------------------------------------------------------------------
//           Name: dx8_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates the three basic types of lights
//                 that are available in Direct3D: directional, spot, and point. 
//
//   Control Keys: F1 - Changes the light's type
//                 F2 - Toggles wire frame mode
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
HWND                    g_hWnd        = NULL;
LPDIRECT3D8             g_pD3D        = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice  = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pWallVB     = NULL; // Vertex buffer for the walls and floor
LPD3DXMESH              g_pSphereMesh = NULL; // A mesh to represent a point light
LPD3DXMESH              g_pConeMesh   = NULL; // A mesh to represent a directional or spot light

const int g_nNumVertsX = 32; // Number of wall vertices along x axis
const int g_nNumVertsZ = 32; // Number of wall vertices along z axis

// Number of triangles needed for the wall
const int g_nTriCount = (g_nNumVertsX-1)*(g_nNumVertsZ-1)*2; 

bool g_bRenderInWireFrame = false;

struct Vertex
{
    D3DXVECTOR3 p; // vertex position
    D3DXVECTOR3 n; // vertex normal
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

enum LightTypes
{
    LIGHT_TYPE_DIRECTIONAL = 0,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_POINT,
};

int g_lightType = LIGHT_TYPE_DIRECTIONAL;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);
void initLightsAndGeometry(void);

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
                             "Direct3D (DX8) - Lighting",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
    initLightsAndGeometry();

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
		            ++g_lightType;
		            if(g_lightType > 2)
			            g_lightType = 0;
		        break;

                case VK_F2:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
                    else
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
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
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );

    D3DCAPS8 d3dCaps;
	g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps );

    if( 0 == ( d3dCaps.VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS ) )
    {
        // If D3DVTXPCAPS_POSITIONALLIGHTS is not set, the device does 
        // not support point or spot lights and the demo won't run properly.
        //MessageBox( g_hWnd, "This hardware doesn't support spot or point lights!", 
                    //"Direct3D Lighting", MB_OK );
    }

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3(-9.0f, 9.0f, -9.0f ), // Camera position
                                  &D3DXVECTOR3( 0.0f, -1.5f, 0.0f ),    // Look-at point
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );   // Up vector
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
}

//-----------------------------------------------------------------------------
// Name: initLightsAndGeometry()
// Desc: 
//-----------------------------------------------------------------------------
void initLightsAndGeometry( void )
{
    // Create a square grid (g_nNumVertsX * g_nNumVertsZ) for 
    // rendering the wall...

	Vertex *v;

    g_pd3dDevice->CreateVertexBuffer( g_nTriCount*3*sizeof(Vertex),
                                      D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
                                      D3DPOOL_MANAGED, &g_pWallVB );

    g_pWallVB->Lock( 0, 0, (BYTE**)&v, 0 );

    float dX = 1.0f/(g_nNumVertsX-1);
    float dZ = 1.0f/(g_nNumVertsZ-1);
    int i = 0;

    for( int z = 0; z < (g_nNumVertsZ-1); ++z )
    {
        for( int x = 0; x < (g_nNumVertsX-1); ++x )
        {
            v[i].p = D3DXVECTOR3(10*x*dX, 0.0f, 10*z*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
            v[i].p = D3DXVECTOR3(10*x*dX, 0.0f, 10*(z+1)*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
            v[i].p = D3DXVECTOR3(10*(x+1)*dX, 0.0f, 10*(z+1)*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
            v[i].p = D3DXVECTOR3(10*x*dX, 0.0f, 10*z*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
            v[i].p = D3DXVECTOR3(10*(x+1)*dX, 0.0f, 10*(z+1)*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
            v[i].p = D3DXVECTOR3(10*(x+1)*dX, 0.0f, 10*z*dZ );
            v[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            ++i;
        }
    }

    g_pWallVB->Unlock();

    // Create a sphere mesh to represent a point light.
    D3DXCreateSphere(g_pd3dDevice, 0.25f, 20, 20, &g_pSphereMesh, NULL);

    // Create a cone mesh to represent a directional or spot light.
    D3DXCreateCylinder(g_pd3dDevice, 0.0f, 0.25f, 0.5f, 20, 20, &g_pConeMesh, NULL);

    // Set up a material
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

    // Set light 0 to be a simple, bright directional light to use 
    // on the mesh that will represent light 2
    D3DLIGHT8 light_0;
    ZeroMemory( &light_0, sizeof(D3DLIGHT8) );
    light_0.Type = D3DLIGHT_DIRECTIONAL;
    light_0.Direction = D3DXVECTOR3( 0.5f, -0.5f, 0.5f );
    light_0.Diffuse.r = 1.0f;
    light_0.Diffuse.g = 1.0f;
    light_0.Diffuse.b = 1.0f;
    g_pd3dDevice->SetLight( 0, &light_0 );

    // Set light 1 to be a simple, faint grey directional light so 
    // the walls and floor are slightly different shades of grey
    D3DLIGHT8 light_1;
    ZeroMemory( &light_1, sizeof(D3DLIGHT8) );
    light_1.Type = D3DLIGHT_DIRECTIONAL;
    light_1.Direction = D3DXVECTOR3( 0.3f, -0.5f, 0.2f );
    light_1.Diffuse.r = 0.25f;
    light_1.Diffuse.g = 0.25f;
    light_1.Diffuse.b = 0.25f;
    g_pd3dDevice->SetLight( 1, &light_1 );

    // Light #2 will be the demo light used to light the floor and walls. 
	// It will be set up in render() since its type can be changed at 
    // run-time.
	
	// Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
		D3DCOLOR_COLORVALUE( 0.25, 0.25, 0.25, 1.0 ) );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pWallVB != NULL )
        g_pWallVB->Release();

	if( g_pSphereMesh != NULL )
        g_pSphereMesh->Release();

	if( g_pConeMesh != NULL )
        g_pConeMesh->Release();

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
	//
	// Create light 2 at run-time based on the light type the user has
    // selected.
	//

	static double dStartTime = timeGetTime();
	float fElpasedAppTime = (float)((timeGetTime() - dStartTime) * 0.001);

	float x = sinf( fElpasedAppTime * 2.000f );
    float y = sinf( fElpasedAppTime * 2.246f );
    float z = sinf( fElpasedAppTime * 2.640f );

    D3DLIGHT8 light_2;
	ZeroMemory( &light_2, sizeof(D3DLIGHT8) );

	light_2.Diffuse.r = 1.0f;
    light_2.Diffuse.g = 1.0f;
    light_2.Diffuse.b = 1.0f;
    light_2.Range     = 100.0f;

	//
	// While both Direct3D and OpenGL use the same formula for lighting 
	// attenuation, they call the variables by different names when setting 
	// them through the API. The following two formulas are the same and 
	// only differ by the API names used for each variable.
	//
	// Direct3D:
	//
	// attenuation = 1 / ( Attenuation0 +
	//                     Attenuation1 * d +
	//                     Attenuation2 * d2 )
	//
	// OpenGL:
	//
	// attenuation = 1 / ( GL_CONSTANT_ATTENUATION  +
	//                     GL_LINEAR_ATTENUATION    * d +
	//                     GL_QUADRATIC_ATTENUATION * d2 )
	//
	// Where:  d = Distance from vertex position to light position
	//        d2 = d squared
	//

    switch( g_lightType )
    {
        case LIGHT_TYPE_DIRECTIONAL:
		{
            light_2.Type      = D3DLIGHT_DIRECTIONAL;
            light_2.Direction = D3DXVECTOR3( x, y, z );
		}
        break;

        case LIGHT_TYPE_SPOT:
		{
            light_2.Type         = D3DLIGHT_SPOT;
            light_2.Position     = 2.0f * D3DXVECTOR3( x, y, z );
            light_2.Direction    = D3DXVECTOR3( x, y, z );
            light_2.Theta        = 0.5f;
            light_2.Phi          = 1.0f;
            light_2.Falloff      = 1.0f;
            light_2.Attenuation0 = 1.0f;
		}
        break;

        case LIGHT_TYPE_POINT:
		{
            light_2.Type         = D3DLIGHT_POINT;
            light_2.Position     = 4.5f * D3DXVECTOR3( x, y, z );
            light_2.Attenuation1 = 0.4f;
		}
        break;
    }

    g_pd3dDevice->SetLight( 2, &light_2 );

	//
	// Render...
	//

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	// Begin the scene
    g_pd3dDevice->BeginScene();

    g_pd3dDevice->SetStreamSource( 0, g_pWallVB, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );

    D3DXMATRIX matWorld;
    D3DXMATRIX matTrans;
    D3DXMATRIX matRotate;

    // The first thing we draw is our walls. The walls are to be lit
    // by lights 1 and 2 only, so we need to turn on lights 1 and 2, 
    // and turn off light 0. Light 0 will be used later for the meshes.
    g_pd3dDevice->LightEnable( 0, FALSE );
    g_pd3dDevice->LightEnable( 1, TRUE );
    g_pd3dDevice->LightEnable( 2, TRUE );

    // Draw the floor
    D3DXMatrixTranslation( &matTrans, -5.0f, -5.0f, -5.0f );
    D3DXMatrixRotationZ( &matRotate, 0.0f );
    matWorld = matRotate * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTriCount );

    // Draw the back wall
    D3DXMatrixTranslation( &matTrans, 5.0f,-5.0f, -5.0f );
    D3DXMatrixRotationZ( &matRotate, D3DXToRadian(90.0) );
    matWorld = matRotate * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTriCount );

    // Draw the side wall
    D3DXMatrixTranslation( &matTrans, -5.0f, -5.0f, 5.0f );
    D3DXMatrixRotationX( &matRotate,  -D3DXToRadian(90.0) );
    matWorld = matRotate * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTriCount );

    // We're finished drawing the walls, we'll now draw the mesh that 
    // represents the light's type. We'll use a little cone for a 
    // directional or spot light and a little sphere for a point light.
    // Light 0 is just for our little meshes, so turn on light 0, and 
    // turn off lights 1 and 2 before rendering.

    g_pd3dDevice->LightEnable( 0, TRUE );
    g_pd3dDevice->LightEnable( 1, FALSE );
    g_pd3dDevice->LightEnable( 2, FALSE );

    // Draw the correct mesh representing the current light type...
    if( g_lightType == LIGHT_TYPE_POINT )
    {
        // Just position the point light - no need to orient it
        D3DXMatrixTranslation( &matWorld, light_2.Position.x, 
            light_2.Position.y, light_2.Position.z );
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        g_pSphereMesh->DrawSubset(0);
    }
	else
    {
        // Position the light and then point it in the light's direction
        D3DXVECTOR3 vecFrom( light_2.Position.x, light_2.Position.y, light_2.Position.z );
        D3DXVECTOR3 vecAt( light_2.Position.x + light_2.Direction.x, 
                           light_2.Position.y + light_2.Direction.y,
                           light_2.Position.z + light_2.Direction.z );
        D3DXVECTOR3 vecUp( 0.0f, 1.0f, 0.0f );
        D3DXMATRIX matWorldInv;
        D3DXMatrixLookAtLH( &matWorldInv, &vecFrom, &vecAt, &vecUp);
        D3DXMatrixInverse( &matWorld, NULL, &matWorldInv);
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        g_pConeMesh->DrawSubset(0);
    }

    // End the scene.
    g_pd3dDevice->EndScene();

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
