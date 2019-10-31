//-----------------------------------------------------------------------------
//           Name: dx9_mesh_attributes.cpp
//         Author: Kevin Harris
//  Last Modified: 05/22/05
//    Description: This sample demonstrates how to manually load the vertex 
//                 buffer of a single ID3DXMesh object with three separate 
//                 colored quads. We then use an attribute table to ID these 
//                 three quads so we can render them separately as subsets of 
//                 the mesh using DrawSubset.
//
//                 The sample also demonstrates how to use the ID3DXMesh methods 
//                 GenerateAdjacency and OptimizeInplace to optimize our test
//                 mesh.
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dndrive/html/directx11192002.asp
//
//   Control Keys: Left Mouse Button - Spin the view
//                 F1 - Toggle rendering of subset #0 of mesh object
//                 F2 - Toggle rendering of subset #1 of mesh object
//                 F3 - Toggle rendering of subset #2 of mesh object
// ----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
using namespace std;
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd       = NULL;
LPDIRECT3D9       g_pD3D       = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPD3DXMESH        g_pMesh      = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

bool g_bRenderSubset0 = true;
bool g_bRenderSubset1 = true;
bool g_bRenderSubset2 = true;

struct Vertex
{
	Vertex(float _x, float _y, float _z, DWORD _color)
	{
        x = _x;  
        y = _y;   
        z = _z;
        color = _color;
	}

    float x, y, z;
    DWORD color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE 
	};
};

//
// Define three quads and create each one from two separate triangles. 
// Using two separate triangles will give us redundant or repeated vertices, 
// which is bad practice, but this will let us test out the OptimizeInplace
// method of ID3DXMesh.
//
Vertex g_verticesFor3Quads[] =
{
   //        x     y     z       color

   // First triangle of quad #1
   Vertex( -1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),
   Vertex(  1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),
   Vertex(  1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),
   // Second triangle of quad #1
   Vertex( -1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),
   Vertex(  1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),
   Vertex( -1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) ),

   // First triangle of quad #2
   Vertex( 1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),
   Vertex( 3.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),
   Vertex( 3.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),
   // Second triangle of quad #2
   Vertex( 1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),
   Vertex( 3.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),
   Vertex( 1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) ),

   // First triangle of quad #3
   Vertex( -3.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) ),
   Vertex( -1.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) ),
   Vertex( -1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) ),
   // Second triangle of quad #3
   Vertex( -3.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) ),
   Vertex( -1.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) ),
   Vertex( -3.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) )
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void redirectIOToConsole(void);
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
    redirectIOToConsole();

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
                             "Direct3D (DX9) - Using Mesh Attributes to Render SubSets",
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

                case VK_F1:
                    g_bRenderSubset0 = !g_bRenderSubset0;
                    break;

                case VK_F2:
                    g_bRenderSubset1 = !g_bRenderSubset1;
                    break;

                case VK_F3:
                    g_bRenderSubset2 = !g_bRenderSubset2;
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
// Name: redirectIOToConsole()
// Desc: 
//-----------------------------------------------------------------------------
void redirectIOToConsole( void )
{
    // Allocate a console so we can output some useful information.
    AllocConsole();

    // Get the handle for STDOUT's file system.
    HANDLE stdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    // Redirect STDOUT to the new console by associating STDOUT's file 
    // descriptor with an existing operating-system file handle.
    int hConsoleHandle = _open_osfhandle( (intptr_t)stdOutputHandle, _O_TEXT );
    FILE *pFile = _fdopen( hConsoleHandle, "w" );
    *stdout = *pFile;
    setvbuf( stdout, NULL, _IONBF, 0 );

    // This call ensures that iostream and C run-time library operations occur  
    // in the order that they appear in source code.
    ios::sync_with_stdio();
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

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    //
    // Create a mesh container...
    //

    HRESULT hr = 0;

	hr = D3DXCreateMeshFVF( 6,  // Our 3 quads have a total of 6 faces or triangles (3 quads * 2 triangles each)
                            18, // which will require 18 vertices to define them.
                            D3DXMESH_MANAGED, 
                            Vertex::FVF_Flags, // This is how each vertex is laid out. 
                            g_pd3dDevice,
                            &g_pMesh );

    //
	// Load our vertices...
	//

	Vertex* pVertices = NULL;

	g_pMesh->LockVertexBuffer( 0, (void**)&pVertices );
    {
        // First triangle of quad #1
        pVertices[0] = g_verticesFor3Quads[0];
	    pVertices[1] = g_verticesFor3Quads[1];
	    pVertices[2] = g_verticesFor3Quads[2];
        // Second triangle of quad #1
	    pVertices[3] = g_verticesFor3Quads[3];
        pVertices[4] = g_verticesFor3Quads[4];
	    pVertices[5] = g_verticesFor3Quads[5];

        // First triangle of quad #2
        pVertices[6] = g_verticesFor3Quads[6];
	    pVertices[7] = g_verticesFor3Quads[7];
	    pVertices[8] = g_verticesFor3Quads[8];
        // Second triangle of quad #2
	    pVertices[9]  = g_verticesFor3Quads[9];
        pVertices[10] = g_verticesFor3Quads[10];
	    pVertices[11] = g_verticesFor3Quads[11];

        // First triangle of quad #3
        pVertices[12] = g_verticesFor3Quads[12];
	    pVertices[13] = g_verticesFor3Quads[13];
	    pVertices[14] = g_verticesFor3Quads[14];
        // Second triangle of quad #3
	    pVertices[15] = g_verticesFor3Quads[15];
        pVertices[16] = g_verticesFor3Quads[16];
	    pVertices[17] = g_verticesFor3Quads[17];

        // If you wanted, you could also memcpy the whole thing in like so.
        //memcpy( pVertices, g_verticesFor3Quads, sizeof(g_verticesFor3Quads) );
    }
	g_pMesh->UnlockVertexBuffer();

	//
	// Define our indices...
	//

	WORD* pIndices = NULL;

	g_pMesh->LockIndexBuffer( 0, (void**)&pIndices );
    {
        // First triangle of quad #1
	    pIndices[0] = 0; 
        pIndices[1] = 1; 
        pIndices[2] = 2;
        // Second triangle of quad #1
	    pIndices[3] = 3; 
        pIndices[4] = 4; 
        pIndices[5] = 5;

        // First triangle of quad #2
	    pIndices[6] = 6; 
        pIndices[7] = 7; 
        pIndices[8] = 8;
        // Second triangle of quad #2
	    pIndices[9]  = 9; 
        pIndices[10] = 10; 
        pIndices[11] = 11;

        // First triangle of quad #3
	    pIndices[12] = 12; 
        pIndices[13] = 13; 
        pIndices[14] = 14;
        // Second triangle of quad #3
	    pIndices[15] = 15; 
        pIndices[16] = 16; 
        pIndices[17] = 17;
    }
	g_pMesh->UnlockIndexBuffer();

    //
	// Now, if you would like to render each quad separately, you can use an 
    // attribute buffer to ID them. We'll use ID #0 for the first quad, ID #1 
    // for second quad, and ID #2 for the third quad. This way, we can use 
    // DrawSubset to pick which quad we want to render. This is very useful if
    // our quads use different textures, or materials which need to be applied 
    // to each one before rendering. Otherwise, we would not have an 
    // opportunity to switch textures or materials once the mesh object starts 
    // rendering them.
    //
    // NOTE: Keep in mind that defining attributes hinders our ability to 
    //       optimize the mesh with a call to OptimizeInplace. This is because
    //       attributes are meant to isolate one subset of a mesh from another
    //       subset for rendering purposes, and vertices can't be shared among 
    //       subsets. Therefore, even if a mesh can be rearranged into a 
    //       nice long tri-strip, which can be rendered in one call, it will 
    //       have to be broken into multiple pieces to support calling 
    //       DrawSubset with a attribute ID.
    //

	DWORD* pAttributes = NULL;
	g_pMesh->LockAttributeBuffer( 0, &pAttributes );
    {
	    pAttributes[0] = 0; // First face or triangle of quad #1
        pAttributes[1] = 0; // First face or triangle of quad #1

        pAttributes[2] = 1; // First face or triangle of quad #2
	    pAttributes[3] = 1; // First face or triangle of quad #2

        pAttributes[4] = 2; // First face or triangle of quad #3
        pAttributes[5] = 2; // First face or triangle of quad #3
    }
	g_pMesh->UnlockAttributeBuffer();

    //
    // Before we get started, dump the indices so we can see them before they 
    // get rearranged.
    //

    pIndices = NULL;
    g_pMesh->LockIndexBuffer( 0, (void**)&pIndices );
    {
        cout << endl;
        cout << "Indices before call to OptimizeInplace" << endl << endl;
        for( int i = 0; i < 18; ++i )
            cout << "index #" << i << " = " << pIndices[i] << endl;
    }
    g_pMesh->UnlockIndexBuffer();

    //
	// The last step for our mesh object is to optimize the mesh. To assist in
    // the optimization step, we need to create a special buffer to hold the
    // adjacency information concerning each triangle face present in the mesh.
    // Since each triangle can have up to 3 neighbors, which are adjacent, we 
    // need a buffer of size (numFaces * 3).
	//

    // Create an adjacency buffer
    LPD3DXBUFFER pAdjacencyBuffer = NULL;
    D3DXCreateBuffer( g_pMesh->GetNumFaces() * 3, &pAdjacencyBuffer );

    // Using a vertex positional tolerance of 0.0f, ask the mesh for its
    // adjacency information.
	g_pMesh->GenerateAdjacency( 0.0f, (DWORD*)pAdjacencyBuffer->GetBufferPointer() );

    // Now, tell the mesh to attempt an optimization based on that 
    // adjacency information.
	hr = g_pMesh->OptimizeInplace( //D3DXMESHOPT_ATTRSORT |    // Reorders faces to optimize for fewer attribute bundle state changes and enhanced ID3DXBaseMesh::DrawSubset performance.
                                   //D3DXMESHOPT_COMPACT |     // Reorders faces to remove unused vertices and faces.
                                   D3DXMESHOPT_VERTEXCACHE | // Reorders faces to increase the cache hit rate of vertex caches.
                                   D3DXMESHOPT_IGNOREVERTS,  // Optimize the faces only; do not optimize the vertices.
		                           (DWORD*)pAdjacencyBuffer->GetBufferPointer(), 
                                   NULL, NULL, NULL );

    if( pAdjacencyBuffer != NULL )
        pAdjacencyBuffer->Release();

    //
    // Dump the indices again to see how they've been rearranged.
    //

    pIndices = NULL;
    g_pMesh->LockIndexBuffer( 0, (void**)&pIndices );
    {
        cout << endl;
        cout << "Indices after call to OptimizeInplace" << endl << endl;
        for( int i = 0; i < 18; ++i )
            cout << "index #" << i << " = " << pIndices[i] << endl;
    }
    g_pMesh->UnlockIndexBuffer();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pMesh != NULL )
        g_pMesh->Release();

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
                         D3DCOLOR_COLORVALUE(0.35f, 0.53f, 0.7, 1.0f), 1.0f, 0 );

    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;
	D3DXMATRIX matWorld;

	D3DXMatrixRotationYawPitchRoll( &matRot,
		                            D3DXToRadian(g_fSpinX),
		                            D3DXToRadian(g_fSpinY),
		                            0.0f );

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 6.5f );

    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

	//
    // Now, lets render the three quads in our mesh via their attribute IDs.
    //

    if( g_bRenderSubset0 == true )
        g_pMesh->DrawSubset(0);

    if( g_bRenderSubset1 == true )
        g_pMesh->DrawSubset(1);

    if( g_bRenderSubset2 == true )
        g_pMesh->DrawSubset(2);

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
