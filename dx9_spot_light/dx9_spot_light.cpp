//-----------------------------------------------------------------------------
//           Name: dx9_spot_light.cpp
//         Author: Kevin Harris
//  Last Modified: 02/17/05
//    Description: This sample demonstrates how to setup a spot light with 
//                 Direct3D. The sample also demonstrates how the tessellation 
//                 or triangle count of a mesh effects lighting.
//
//   Control Keys: v - Decrease the mesh's tessellation or vertex count
//                 V - Increase the mesh's tessellation or vertex count
//                 w - Toggle wire-frame mode
//
//                 r  - Decrease Range
//                 R  - Increase Range
//                 t  - Decrease Theta
//                 T  - Increase Theta
//                 p  - Decrease Phi
//                 P  - Increase Phi
//                 f  - Decrease Falloff
//                 F  - Increase Falloff
//                 F1 - Decrease Attenuation0
//                 F2 - Increase Attenuation0
//                 F3 - Decrease Attenuation1
//                 F4 - Increase Attenuation1
//                 F5 - Decrease Attenuation2
//                 F6 - Increase Attenuation2
//
//                 Up    - Move the spot light up
//                 Down  - Move the spot light down
//                 Left  - Swing the spot to the left
//                 Right - Swing the spot to the right
//
// NOTE: Please refer to the DirectX 9.0c Documentation for the more 
//       information concerning the Attenuation, Theta, Phi, and Falloff 
//       controls.
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
LPDIRECT3DVERTEXBUFFER9 g_pMeshVertices = NULL; // Vertex buffer for our custom made mesh
LPD3DXMESH              g_pConeMesh     = NULL; // A mesh to represent a spot light
D3DLIGHT9               g_light0;
D3DMATERIAL9            g_meshMaterial;
D3DMATERIAL9            g_coneMaterial;

bool  g_bRenderInWireFrame = false;
float g_fLightPositionY    = 3.0f;
float g_fLightDirectionX   = 0.0f;
float g_fAttenuation0      = 0.0f;
float g_fAttenuation1      = 0.5f;
float g_fAttenuation2      = 0.0f;
float g_fRange             = 10.0f;
float g_fTheta             = 0.5f;
float g_fPhi               = 1.0f;
float g_fFalloff           = 1.0f;
int   g_nMeshVertCount     = 0;
int   g_nNumVerts          = 64;

#define D3DFVF_MY_VERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE )

struct Vertex
{
	float x, y, z;    // Position of vertex in 3D space
    float nx, ny, nz; // Normal for lighting calculations
    DWORD diffuse;    // Diffuse color of vertex
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
void initLighting(void);
int createSimpleMesh(int nNumVertsAlongX, int nNumVertsAlongZ, 
                     float fMeshLengthAlongX, float fMeshLengthAlongZ);

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

	if(!RegisterClassEx( &winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", "Direct3D (DX9) - Spot Light",
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
    switch( msg )
	{	
        case WM_CHAR:
		{
			switch( wParam )
			{
                case 'v':
                    g_nNumVerts -= 1;

                    // Don't let the mesh's resolution get too low!
                    if( g_nNumVerts <= 1 )
                        g_nNumVerts = 2;

                    // Release the old mesh so we can create a new one.
                    if( g_pMeshVertices != NULL )
                        g_pMeshVertices->Release();

                    g_nMeshVertCount = createSimpleMesh( g_nNumVerts, g_nNumVerts, 10.0f, 10.0f );
                    break;

                case 'V':
                    g_nNumVerts += 1;

                    // Release the old mesh so we can create a new one.
                    if( g_pMeshVertices != NULL )
                        g_pMeshVertices->Release();

                    g_nMeshVertCount = createSimpleMesh( g_nNumVerts, g_nNumVerts, 10.0f, 10.0f );
                    break;

                case 'r':
                    g_fRange -= 1.0f;
                    break;

                case 'R':
                    g_fRange += 1.0f;
                    break;

                case 't':
                    g_fTheta -= 0.01f;

                    if( g_fTheta < 0.0f )
                        g_fTheta = 0.0f;
                    break;

                case 'T':
                    g_fTheta += 0.01f;
                    break;

                case 'p':
                    g_fPhi -= 0.01f;

                    if( g_fPhi < 0.0f )
                        g_fPhi = 0.0f;
                    break;

                case 'P':
                    g_fPhi += 0.01f;
                    break;

                case 'f':
                    g_fFalloff -= 0.01f;

                    if( g_fFalloff < 0.0f )
                        g_fFalloff = 0.0f;
                    break;

                case 'F':
                    g_fFalloff += 0.01f;
                    break;

                case 'w':
                case 'W':
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
                    else
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
                    break;
            }
        }
        break;

        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

                case VK_F1:
                    g_fAttenuation0 -= 0.01f;

                    if( g_fAttenuation0 < 0.0f )
                        g_fAttenuation0 = 0.0f;
                    break;

                case VK_F2:
                    g_fAttenuation0 += 0.01f;
                    break;

                case VK_F3:
                    g_fAttenuation1 -= 0.01f;

                    if( g_fAttenuation1 < 0.0f )
                        g_fAttenuation1 = 0.0f;
                    break;

                case VK_F4:
                    g_fAttenuation1 += 0.01f;
                    break;

                case VK_F5:
                    g_fAttenuation2 -= 0.01f;

                    if( g_fAttenuation2 < 0.0f )
                        g_fAttenuation2 = 0.0f;
                    break;

                case VK_F6:
                    g_fAttenuation2 += 0.01f;
                    break;

                 case VK_UP:
                    g_fLightPositionY += 0.1f;
                    break;

                case VK_DOWN:
                    g_fLightPositionY -= 0.1f;
                    break;

                case VK_LEFT:
                    g_fLightDirectionX -= 0.1f;
                    break;

                case VK_RIGHT:
                    g_fLightDirectionX += 0.1f;
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
// Desc: Initializes Direct3D under DirectX 9.0
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

	D3DXMATRIX mProjection;
    D3DXMatrixPerspectiveFovLH( &mProjection, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProjection );

    D3DXMATRIX mView;
    D3DXMatrixLookAtLH( &mView, &D3DXVECTOR3( 0.0f, 4.0f, -14.0f ), // Camera position
                                &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),   // Look-at point
                                &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) ); // Up vector
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &mView );

    initLighting();

    // Create a custom mesh to test lighting on.
    g_nMeshVertCount = createSimpleMesh( g_nNumVerts, g_nNumVerts, 10.0f, 10.0f );

    // Create a sphere mesh to represent our spot light's position and direction.
    D3DXCreateCylinder( g_pd3dDevice, 0.0f, 0.25f, 0.5f, 20, 20, &g_pConeMesh, NULL );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release all Direct3D resources.
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pMeshVertices != NULL )
        g_pMeshVertices->Release(); 

    if( g_pConeMesh != NULL )
        g_pConeMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: initLighting()
// Desc: 
//-----------------------------------------------------------------------------
void initLighting( void )
{
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->LightEnable( 0, TRUE );

    // Set up our material...
	ZeroMemory( &g_meshMaterial, sizeof(D3DMATERIAL9) );
    g_meshMaterial.Diffuse.r = 1.0f;
    g_meshMaterial.Diffuse.g = 1.0f;
    g_meshMaterial.Diffuse.b = 1.0f;
    g_meshMaterial.Diffuse.a = 1.0f;
    g_meshMaterial.Ambient.r = 1.0f;
    g_meshMaterial.Ambient.g = 1.0f;
    g_meshMaterial.Ambient.b = 1.0f;
    g_meshMaterial.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &g_meshMaterial );

    // Set up our spot light...
	ZeroMemory( &g_light0, sizeof(D3DLIGHT9) );
	g_light0.Diffuse.r    = 1.0f;
    g_light0.Diffuse.g    = 1.0f;
    g_light0.Diffuse.b    = 1.0f;
    g_light0.Diffuse.a    = 1.0f;
    g_light0.Type         = D3DLIGHT_SPOT;
    g_light0.Position     = D3DXVECTOR3( 0.0f, g_fLightPositionY, 0.0f );
    g_light0.Direction    = D3DXVECTOR3( g_fLightDirectionX, -1.0, -0.1f );
    g_light0.Attenuation0 = g_fAttenuation0;
    g_light0.Attenuation1 = g_fAttenuation1;
    g_light0.Attenuation2 = g_fAttenuation2;
    g_light0.Range        = g_fRange;
    g_light0.Theta        = g_fTheta;
    g_light0.Phi          = g_fPhi;
    g_light0.Falloff      = g_fFalloff;
    g_pd3dDevice->SetLight( 0, &g_light0 );

    // Enable some dim, grey ambient lighting...
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,
		                          D3DCOLOR_COLORVALUE( 0.25, 0.25, 0.25, 1.0 ) );

    // Set up a yellow emissive material so our little sphere, which 
    // represents our light, will appear to glow yellow.
    ZeroMemory( &g_coneMaterial, sizeof(D3DMATERIAL9) );
    g_coneMaterial.Emissive.r = 1.0f;
    g_coneMaterial.Emissive.g = 1.0f;
    g_coneMaterial.Emissive.b = 0.0f;
    g_coneMaterial.Emissive.a = 1.0f;
}

//-----------------------------------------------------------------------------
// Name: createSimpleMesh()
// Desc: 
//-----------------------------------------------------------------------------
int createSimpleMesh( int nNumVertsAlongX, int nNumVertsAlongZ, 
                       float fMeshLengthAlongX, float fMeshLengthAlongZ  )
{
    int nMeshVertCount = (nNumVertsAlongX-1) * (nNumVertsAlongZ-1) * 6;

    // Compute position deltas for moving down the X, and Z axis during mesh creation
	const float dX = 1.0f/(nNumVertsAlongX-1);
	const float dZ = 1.0f/(nNumVertsAlongZ-1);

    g_pd3dDevice->CreateVertexBuffer( nMeshVertCount*sizeof(Vertex),
                                      D3DUSAGE_WRITEONLY, D3DFVF_MY_VERTEX,
                                      D3DPOOL_MANAGED, &g_pMeshVertices, NULL );
    int i = 0;
	int x = 0;
	int z = 0;

    Vertex *v;
    g_pMeshVertices->Lock( 0, 0, (void**)&v, 0 );

    // These are all the same...
	for( i = 0; i < nMeshVertCount; ++i )
	{
        // Mesh tesselation occurs in the X,Z plane, so Y is always zero
		v[i].ny = 1.0f;
        
        v[i].nx = 0.0f;
		v[i].ny = 1.0f;
		v[i].nz = 0.0f;
        
        v[i].diffuse = D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f);
	}

    //
	// Create all the vertex points required by the mesh...
	//
	// Note: Mesh tesselation occurs in the X,Z plane.
	//
    
    // For each row of our mesh...
    for( z = 0, i = 0; z < (nNumVertsAlongZ-1); ++z )
    {
        // Fill the row with quads which are composed of two triangles each...
        for( x = 0; x < (nNumVertsAlongX-1); ++x )
        {
            // First triangle of the current quad
			// 1 ___ 2
			//  |  /|
			//  |/__|
			// 0

            // 0
            v[i].x = fMeshLengthAlongX * x * dX;
            v[i].z = fMeshLengthAlongZ * z * dZ;
            ++i;

            // 1
            v[i].x = fMeshLengthAlongX * x * dX;
            v[i].z = fMeshLengthAlongZ * (z+1) * dZ;
            ++i;

            // 2
            v[i].x = fMeshLengthAlongX * (x+1) * dX;
            v[i].z = fMeshLengthAlongZ * (z+1) * dZ;
            ++i;

            // Second triangle of the current quad
			//   ___ 1
			//  |  /|
			//  |/__|
			// 0     2

            // 0
            v[i].x = fMeshLengthAlongX * x * dX; 
            v[i].z = fMeshLengthAlongZ * z * dZ;
            ++i;

            // 1
            v[i].x = fMeshLengthAlongX * (x+1) * dX;
            v[i].z = fMeshLengthAlongZ * (z+1) * dZ;
            ++i;

            // 2
            v[i].x = fMeshLengthAlongX * (x+1) * dX;  
            v[i].z = fMeshLengthAlongZ * z * dZ;
            ++i;
        }
    }

    g_pMeshVertices->Unlock();

    return nMeshVertCount;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render or draw our scene to the monitor.
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	g_pd3dDevice->BeginScene();

    D3DXMATRIX matTrans;
    D3DXMATRIX matRotate;
    D3DXMATRIX matWorld;

    //
    // Set up our light and apply a material...
    //

    g_light0.Position.y   = g_fLightPositionY;
    g_light0.Direction.x  = g_fLightDirectionX;
    g_light0.Attenuation0 = g_fAttenuation0;
    g_light0.Attenuation1 = g_fAttenuation1;
    g_light0.Attenuation2 = g_fAttenuation2;
    g_light0.Range        = g_fRange;
    g_light0.Theta        = g_fTheta;
    g_light0.Phi          = g_fPhi;
    g_light0.Falloff      = g_fFalloff;

    g_pd3dDevice->SetLight( 0, &g_light0 );
    g_pd3dDevice->LightEnable( 0, TRUE );
    g_pd3dDevice->SetMaterial( &g_meshMaterial );

    //
    // Render a tesselated mesh of variable resolution so we can see how its 
    // tessleation alters our attempt to light it.
    //

    D3DXMatrixTranslation( &matTrans, -5.0f, 0.0f, -5.0f );
    D3DXMatrixRotationZ( &matRotate, D3DXToRadian(0.0f) );
                         matWorld = matRotate * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->SetStreamSource( 0, g_pMeshVertices, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nMeshVertCount/3 );

    //
    // Render a little yellow cone to represent out spot light's current  
    // position and direction in 3D space.
    //

    g_pd3dDevice->LightEnable( 0, FALSE );
    g_pd3dDevice->SetMaterial( &g_coneMaterial );

    // Position the spot light and then point it in the light's direction
    D3DXVECTOR3 vecFrom( g_light0.Position.x, g_light0.Position.y, g_light0.Position.z );
    D3DXVECTOR3 vecAt( g_light0.Position.x + g_light0.Direction.x,
                       g_light0.Position.y + g_light0.Direction.y,
                       g_light0.Position.z + g_light0.Direction.z );
    D3DXVECTOR3 vecUp( 0.0f, 1.0f, 0.0f );
    D3DXMATRIX matWorldInv;
    D3DXMatrixLookAtLH( &matWorldInv, &vecFrom, &vecAt, &vecUp);
    D3DXMatrixInverse( &matWorld, NULL, &matWorldInv);
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pConeMesh->DrawSubset(0);

	g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
