//-----------------------------------------------------------------------------
//           Name: ogl_cg_displacement_mapping.cpp
//         Author: Kevin Harris
//  Last Modified: 06/10/05
//    Description: This sample demonstrates how to perform displacement mapping
//                 using Cg. The sample requires support for CG_PROFILE_VP40, 
//                 which basically means that your card needs to support 
//                 GL_ARB_vertex_program and GL_NV_vertex_program3.
//
//   Control Keys: s - Toggle usage of displacement shader.
//                 w - Toggle wire-frame mode.
//                 d - Increase displacement.
//                 D - Decrease displacement.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_decalTextureID = 0;
GLuint    g_displacementTextureID = 0;

CGprofile   g_CGprofile_vertex;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram_vertex;
CGparameter g_CGparam_displacementTexture;
CGparameter g_CGparam_displacementScaler;

bool  g_bUseShaders = true;
bool  g_bWireFrameMode = false;
float g_fDisplacementScaler = 0.5f;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
	// GL_T2F_C4F_N3F_V3F
	float tu, tv;
	float r, g, b, a;
	float nx, ny, nz;
	float x, y, z;
};

//
// Mesh properties...
//

const int   g_nNumVertsAlongX   = 64;
const int   g_nNumVertsAlongZ   = 64;
const float g_fMeshLengthAlongX = 2.0f;
const float g_fMeshLengthAlongZ = 2.0f;

//
// -- Regular mesh --
//
// Composed of simple triangles.
//

// Number of vertices required for the mesh
const int g_nRegularVertCount = (g_nNumVertsAlongX-1) * (g_nNumVertsAlongZ-1) * 6;
Vertex g_meshVertices[g_nRegularVertCount];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTextures(void);
void init(void);
void render(void);
void shutDown(void);
void initShader(void);
void createMesh(void);

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
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Simple Displacement Mapping Using Cg",
							WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initShader();

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

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
    static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
    
    switch( msg )
	{
		case WM_CHAR:
		{
			switch( wParam )
			{
				case 's':
				case 'S':
					g_bUseShaders = !g_bUseShaders;
					break;

				case 'w':
				case 'W':
					g_bWireFrameMode = !g_bWireFrameMode;
					break;

				case 'd':
					g_fDisplacementScaler += 0.1f;
					break;

				case 'D':
					g_fDisplacementScaler -= 0.1f;
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

		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
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
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadTextures()
// Desc: 
//-----------------------------------------------------------------------------
void loadTextures( void )
{
	//
	// Load the displacement map as a float texture. The displacement map is 
	// basically just a gray-scale height map, Later, we will pass this to the 
	// vertex shader where it will be used to displace the vertices.
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\nveye_displace.bmp" );

	if( pTextureImage != NULL )
	{
		glGenTextures( 1, &g_displacementTextureID );

		glBindTexture( GL_TEXTURE_2D, g_displacementTextureID );

		//
		// Note the GL_NEAREST mip-map settings and the float-texture format of 
		// GL_RGBA_FLOAT32_ATI used by our displacement map.
		//

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA_FLOAT32_ATI, pTextureImage->sizeX, 
			pTextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE,  pTextureImage->data );
	}

	if( pTextureImage )
	{
		if( pTextureImage->data )
			free( pTextureImage->data );

		free( pTextureImage );
	}

	//
	// Load a regular decal texture...
	//

	pTextureImage = auxDIBImageLoad( ".\\nveye.bmp" );

	if( pTextureImage != NULL )
	{
		glGenTextures( 1, &g_decalTextureID );

		glBindTexture( GL_TEXTURE_2D, g_decalTextureID );

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0,
			GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
	}

	if( pTextureImage )
	{
		if( pTextureImage->data )
			free( pTextureImage->data );

		free( pTextureImage );
	}
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd );
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

	loadTextures();
	createMesh();
}

//-----------------------------------------------------------------------------
// Name: createMesh()
// Desc: 
//-----------------------------------------------------------------------------
void createMesh( void )
{
	// Compute position deltas for moving down the X, and Z axis during mesh creation
	const float dX =  (1.0f/(g_nNumVertsAlongX-1));
	const float dZ = -(1.0f/(g_nNumVertsAlongZ-1));

	// Compute tex-coord deltas for moving down the X, and Z axis during mesh creation
	const float dTU = 1.0f/(g_nNumVertsAlongX-1);
	const float dTV = 1.0f/(g_nNumVertsAlongZ-1);

	int i = 0;
	int x = 0;
	int z = 0;

	// These are all the same...
	for( i = 0; i < g_nRegularVertCount; ++i )
	{
		// Mesh tesselation occurs in the X,Z plane, so Y is always zero
		g_meshVertices[i].y = 0.0f;

		g_meshVertices[i].nx = 0.0f;
		g_meshVertices[i].ny = 1.0f;
		g_meshVertices[i].nz = 0.0f;

		g_meshVertices[i].r = 1.0f;
		g_meshVertices[i].g = 1.0f;
		g_meshVertices[i].b = 1.0f;
	}

	//
	// Create all the vertex points required by the mesh...
	//
	// Note: Mesh tesselation occurs in the X,Z plane.
	//

	// For each row of our mesh...
	for( z = 0, i = 0; z < (g_nNumVertsAlongZ-1); ++z )
	{
		// Fill the row with quads which are composed of two triangles each...
		for( x = 0; x < (g_nNumVertsAlongX-1); ++x )
		{
			// First triangle of the current quad
			//   ___ 2
			//  |  /|
			//  |/__|
			// 0     1

			// 0
			g_meshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_meshVertices[i].tu = x * dTU;
			g_meshVertices[i].tv = z * dTV;
			++i;

			// 1
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_meshVertices[i].tu = (x+1.0f) * dTU;
			g_meshVertices[i].tv = z * dTV;
			++i;

			// 2
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_meshVertices[i].tu = (x+1.0f) * dTU;
			g_meshVertices[i].tv = (z+1.0f) * dTV;
			++i;

			// Second triangle of the current quad
			// 2 ___ 1
			//  |  /|
			//  |/__|
			// 0

			// 0
			g_meshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_meshVertices[i].tu = x * dTU;
			g_meshVertices[i].tv = z * dTV;
			++i;

			// 1
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_meshVertices[i].tu = (x+1.0f) * dTU;
			g_meshVertices[i].tv = (z+1.0f) * dTV;
			++i;

			// 2
			g_meshVertices[i].x = g_fMeshLengthAlongX * x * dX;
			g_meshVertices[i].z = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_meshVertices[i].tu = x * dTU;
			g_meshVertices[i].tv = (z+1.0f) * dTV;
			++i;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the CG shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
    //
    // Search for a certain vertex shader profile:
    //
    // CG_PROFILE_VP40 - GL_ARB_vertex_program + GL_NV_vertex_program3
    //

	if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
		g_CGprofile_vertex = CG_PROFILE_VP40;
	else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
            "support CG_PROFILE_VP40, which is required to run!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create a displacement shader...
	//
	
	g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext,
										          CG_SOURCE,
										          "ogl_cg_displacement_mapping.cg",
										          g_CGprofile_vertex,
										          NULL, 
										          NULL );

	//
	// Load the programs using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram_vertex );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_displacementTexture = cgGetNamedParameter(g_CGprogram_vertex, "displacementTexture");
	g_CGparam_displacementScaler  = cgGetNamedParameter(g_CGprogram_vertex, "displacementScaler");
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_decalTextureID );
	glDeleteTextures( 1, &g_displacementTextureID );

    cgDestroyProgram( g_CGprogram_vertex );
	cgDestroyContext( g_CGcontext );
        
	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;							
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	if( g_bWireFrameMode )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -3.0f );
	glRotatef( -(g_fSpinY - 90.0f), 1.0f, 0.0f, 0.0f );
	glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );
	// Since the mesh is created off-center in the XZ plane we'll need to move 
	// it to the center for it to spin correctly in the sample.
	glTranslatef( -(g_fMeshLengthAlongX/2.0f), 0.0f, (g_fMeshLengthAlongZ/2.0f) );

	if( g_bUseShaders == true )
	{
		float fdisplacementScaler[] = { g_fDisplacementScaler, 0.0f, 0.0f, 1.0f };
		cgGLSetParameter4fv( g_CGparam_displacementScaler, fdisplacementScaler );

		cgGLEnableTextureParameter( g_CGparam_displacementTexture );
		cgGLSetTextureParameter( g_CGparam_displacementTexture, g_displacementTextureID );

		cgGLBindProgram( g_CGprogram_vertex );
		cgGLEnableProfile( g_CGprofile_vertex );

		// Assign the regular decal texture
		glBindTexture( GL_TEXTURE_2D, g_decalTextureID );
		glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_meshVertices );
		glDrawArrays( GL_TRIANGLES, 0, g_nRegularVertCount );

		cgGLDisableTextureParameter( g_CGparam_displacementTexture );

		cgGLDisableProfile( g_CGprofile_vertex );
	}
	else
	{
		//
		// Render the normal way...
		//

		glBindTexture( GL_TEXTURE_2D, g_decalTextureID );
		glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_meshVertices );
		glDrawArrays( GL_TRIANGLES, 0, g_nRegularVertCount );
	}

	SwapBuffers( g_hDC );
}

