//-----------------------------------------------------------------------------
//           Name: dx8_lookat_matrix.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample shows you how you can use the player's current 
//                 position, the scene's up vector, and the direction that the 
//                 player is currently looking in to create a proper view 
//                 matrix for Diretc3D.
//       Controls: Use the arrow keys to move the imaginary point that the 
//                 player is looking at.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <time.h>
#include <d3dx8.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// MACROS / DEFINES
//-----------------------------------------------------------------------------
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80)

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECT3D8         g_pD3D           = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8   g_pd3dDevice     = NULL; // Our rendering device
LPD3DXMESH          g_pMesh          = NULL; // Our mesh object in sysmem
D3DMATERIAL8       *g_pMeshMaterials = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE8 *g_pMeshTextures  = NULL; // Textures for our mesh
unsigned long       g_dwNumMaterials = 0L;   // Number of mesh materials

float g_fMoveAmount = 0.1f;

D3DXVECTOR3 g_vUpVec( 0.0f, 1.0f, 0.0f );
D3DXVECTOR3 g_vEyePt( 0.0f, 0.0f, 3.0f );
D3DXVECTOR3 g_vLookatPt( 0.0f, 0.0f, 0.0f );

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
void init(HWND hWnd);
void shutDown(void);
HRESULT render(HWND hWnd);
void updateViewMatrix(void);
void buildLookAtMatrix( D3DXMATRIX *pOut, 
					    const D3DXVECTOR3 &pEye, 
					    const D3DXVECTOR3 &pAt, 
					    const D3DXVECTOR3 &pUp );

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

				case VK_UP:
					g_vLookatPt.y += g_fMoveAmount;
					break;

				case VK_DOWN:
					g_vLookatPt.y -= g_fMoveAmount;
					break;

				case VK_LEFT:
					g_vLookatPt.x -= g_fMoveAmount;
					break;

				case VK_RIGHT:
					g_vLookatPt.x += g_fMoveAmount;
					break;

				case VK_HOME:
					g_vLookatPt.y += g_fMoveAmount; 
					break;

				case VK_END:
					g_vLookatPt.y -= g_fMoveAmount;
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
	MSG        msg;

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
						   "Direct3D (DX8) - Look-at Matrix",
                           WS_OVERLAPPEDWINDOW, 0,0, 640,480, NULL, NULL, 
                           hInstance, NULL );

	if( hWnd == NULL )
		return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

	init( hWnd );
	
	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
        else
		    render( hWnd );
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return msg.wParam;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
void init( HWND hWnd )
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

    // Set up some general Direct3D settings...
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE ); // Turn off lighting
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );  // Turn on the z-buffer
	g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME ); // Turn on wire frame

	// Set up the projection
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 500.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
    
	//-------------------------------------------------------------------------

	LPD3DXBUFFER pD3DXMtrlBuffer;

    D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM, 
                       g_pd3dDevice, NULL, 
                       &pD3DXMtrlBuffer, &g_dwNumMaterials, 
                       &g_pMesh );

    // We need to extract the material properties and texture names 
    // from the pD3DXMtrlBuffer
    D3DXMATERIAL *d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_pMeshMaterials = new D3DMATERIAL8[g_dwNumMaterials];
    g_pMeshTextures  = new LPDIRECT3DTEXTURE8[g_dwNumMaterials];

    for( unsigned long i = 0; i < g_dwNumMaterials; ++i )
    {
        // Copy the material over...
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // Set the ambient color for the material (D3DX does not do this)
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
     
		// Create the texture...
		g_pMeshTextures[i] = NULL;
        D3DXCreateTextureFromFile( g_pd3dDevice, d3dxMaterials[i].pTextureFilename, 
                                   &g_pMeshTextures[i] );
    }

    pD3DXMtrlBuffer->Release();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pMeshMaterials != NULL ) 
        delete[] g_pMeshMaterials;

    if( g_pMeshTextures != NULL )
    {
        for( unsigned long i = 0; i < g_dwNumMaterials; ++i )
			SAFE_RELEASE( g_pMeshTextures[i] );
	
        delete[] g_pMeshTextures;
    }

    SAFE_RELEASE( g_pMesh );
	SAFE_RELEASE( g_pd3dDevice );
	SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render the scene
//-----------------------------------------------------------------------------
HRESULT render( HWND hWnd )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                         D3DCOLOR_COLORVALUE( 0.350f, 0.530f, 0.701, 1.0f ), 
                         1.0f, 0 );

    g_pd3dDevice->BeginScene();

	updateViewMatrix();
	
	// Meshes are divided into subsets, one for each material. 
    // Render them in a loop
    for( unsigned long i = 0; i < g_dwNumMaterials; ++i )
    {
        // Set the material and texture for this subset
        g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
        g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );
        
        // Draw the mesh subset
        g_pMesh->DrawSubset( i );
    }

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name : updateViewMatrix()
// Desc : 
//-----------------------------------------------------------------------------
void updateViewMatrix( void )
{  
	D3DXMATRIX matView;

	// Build a look-at matrix using our new function...
	buildLookAtMatrix( &matView, g_vEyePt, g_vLookatPt, g_vUpVec );

	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView ); 
}

//-----------------------------------------------------------------------------
// Name: buildLookAtMatrix()
// Desc: Creates a Left-Handed view matrix
//
// Building a View Matrix:
//
// 1. Subtract the camera's position from the position to look at, to 
//    create a view vector called 'l'.
// 2. Take the cross-product of the new view vector and the y axis of 
//    world space to get a right vector called 'r'.
// 3. The cross-product of vectors 'l' and 'r' will give you a up vector 
//    called 'u'.
// 4. Compute translation factors by taking the negative of the 
//    dot-product between the camera's position, called 'c', and our new 
//    orientation or basis vectors called u, r, and l.
//
// Here's what the final matrix should look like:
//
//  |   rx     ux     lx    0 |
//  |   ry     uy     ly    0 |
//  |   rz     uz     lz    0 |
//  | -(r.c) -(u.c) -(l.c)  1 |
//
// Where r = Right vector
//       u = Up vector
//       l = Look vector
//       c = Camera's world space position
//       . = Dot-product operation
//-----------------------------------------------------------------------------
void buildLookAtMatrix( D3DXMATRIX        *pOut, 
					    const D3DXVECTOR3 &pEye, 
					    const D3DXVECTOR3 &pAt, 
					    const D3DXVECTOR3 &pUp )
{
    D3DXVECTOR3 vRight; // New x basis
	D3DXVECTOR3 vUp;    // New y basis
	D3DXVECTOR3 vView;  // New z basis

    // Subtract the camera's position from the viewer's position to 
    // create a direction or view vector for the camera. We'll call 
	// this vector, vView.
    D3DXVec3Subtract(&vView, &pAt, &pEye);

    // Normalize our new z basis vector.
    D3DXVec3Normalize(&vView, &vView);

    // Take the cross-product of the new direction vector and the 
    // x axis of the world space to get a right vector.
	// We'll call this vector vRight.
    D3DXVec3Cross(&vRight, &pUp, &vView);
    D3DXVec3Normalize(&vRight, &vRight);

	// The cross-product of the direction vector (vView) and the 
	// right vector (vRight) will give us our new up vector, which 
	// we'll call vUp.
    D3DXVec3Cross(&vUp, &vView, &vRight);

    // We now build the matrix. The first three columns contain the 
    // basis vectors used to rotate the view to point at the look-at 
    // point. The fourth row contains the translation values. 
    // Rotations are still about the eye point.
	//
	// m11 m12 m13 m14
	// m21 m22 m23 m24
	// m31 m32 m33 m34
	// m41 m42 m43 m44

    pOut->_11 = vRight.x;
    pOut->_21 = vRight.y;
    pOut->_31 = vRight.z;
    pOut->_41 = -D3DXVec3Dot( &vRight, &pEye );

    pOut->_12 = vUp.x;
    pOut->_22 = vUp.y;
    pOut->_32 = vUp.z;
    pOut->_42 = -D3DXVec3Dot( &vUp, &pEye );

    pOut->_13 = vView.x;
    pOut->_23 = vView.y;
    pOut->_33 = vView.z;
    pOut->_43 = -D3DXVec3Dot( &vView, &pEye );

    pOut->_14 = 0.0f;
    pOut->_24 = 0.0f;
    pOut->_34 = 0.0f;
    pOut->_44 = 1.0f;
}
