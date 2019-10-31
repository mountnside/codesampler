//-----------------------------------------------------------------------------
//           Name: ogl_cg_vertex_displacement.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to perform mesh deformation or 
//                 vertex displacement with OpenGL using a Cg shader.
//
//   Control Keys: F1 - Increase flag motion
//                 F2 - Decrease flag motion
//                 F3 - Toggle wire-frame mode
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC             = NULL;
HGLRC     g_hRC             = NULL;
HWND      g_hWnd            = NULL;
HINSTANCE g_hInstance       = NULL;
GLuint    g_textureID       = -1;
GLuint    g_meshDisplayList = -1;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_currentAngle;

float g_fCurrentAngle    = 0.0f;
float g_fSpeedOfRotation = 10.0f;
bool  g_bWireFrameMode   = false;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;
float  g_fSpinX = 0.0f;
float  g_fSpinY = 0.0f;

// Mesh properties...
const int   g_nNumVertsAlongX   = 16;
const int   g_nNumVertsAlongZ   = 16;
const float g_fMeshLengthAlongX = 8.0f;
const float g_fMeshLengthAlongZ = 5.0f;

const int g_nMeshVertCount = (g_nNumVertsAlongX-1) * (g_nNumVertsAlongZ-1) * 6;

struct Vertex
{
	// GL_T2F_N3F_V3F
	float tu, tv;
	float nx, ny, nz;
	float x, y, z;
};

Vertex g_meshVertices[g_nMeshVertCount];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void createMesh(void);
void render(void);
void shutDown(void);
void initShader(void);
void updateViewMatrix(void);
void setShaderConstants(void);

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
						    "OpenGL - Vertex Displacement Shader Using CG",
                            WS_OVERLAPPEDWINDOW, 0,0, 640,480, NULL, NULL, 
                            g_hInstance, NULL );

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
		{
			g_dCurTime     = timeGetTime();
			g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
			g_dLastTime    = g_dCurTime;

		    render();
		}
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
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

                case VK_F1:
					g_fSpeedOfRotation += 0.5f;
					break;

                case VK_F2:
					g_fSpeedOfRotation -= 0.5f;
					break;

                case VK_F3:
					g_bWireFrameMode = !g_bWireFrameMode;
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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)	
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\us_flag.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

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

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

    loadTexture();
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
	for( i = 0; i < g_nMeshVertCount; ++i )
	{
		// Mesh tesselation occurs in the X,Z plane, so Y is always zero
		g_meshVertices[i].y = 0.0f;

		g_meshVertices[i].nx = 0.0f;
		g_meshVertices[i].ny = 1.0f;
		g_meshVertices[i].nz = 0.0f;
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

    g_meshDisplayList = glGenLists(1);
    glNewList( g_meshDisplayList, GL_COMPILE );
    glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_meshVertices );
    glDrawArrays( GL_TRIANGLES , 0, g_nMeshVertCount );
    glEndList();
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the CG shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
    //
    // Search for a valid vertex shader profile in this order:
    //
    // CG_PROFILE_ARBVP1 - GL_ARB_vertex_program
    // CG_PROFILE_VP40   - GL_ARB_vertex_program + GL_NV_vertex_program3
    //

    if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        g_CGprofile = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
        g_CGprofile = CG_PROFILE_VP40;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
            "support any of the required vertex shading extensions!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create the vertex shader...
	//
	
	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
										   CG_SOURCE,
										   "ogl_cg_vertex_displacement.cg",
										   g_CGprofile,
										   NULL, 
										   NULL );

	//
	// Load the program using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_currentAngle = cgGetNamedParameter( g_CGprogram, "currentAngle" );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );

	cgDestroyProgram(g_CGprogram);
	cgDestroyContext(g_CGcontext);
	
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
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
    g_fCurrentAngle -= g_fSpeedOfRotation * g_fElpasedTime;

    while( g_fCurrentAngle > 360.0f ) g_fCurrentAngle -= 360.0f;
    while( g_fCurrentAngle < 0.0f   ) g_fCurrentAngle += 360.0f;

	// Load a value, which continuously grows from 0 to 360 and repeats...
    cgGLSetParameter4f( g_CGparam_currentAngle, g_fCurrentAngle, 0.0f, 0.0f, 0.0f );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( -4.0f, -2.5f, -10.0f );
    glRotatef( -g_fSpinY + 90.0f, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    setShaderConstants();

    cgGLBindProgram( g_CGprogram );
	cgGLEnableProfile( g_CGprofile );

    if( g_bWireFrameMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    //
    // Render tesselated mesh...
    //

    glBindTexture( GL_TEXTURE_2D, g_textureID );
    glCallList( g_meshDisplayList );

	cgGLDisableProfile( g_CGprofile );

	SwapBuffers( g_hDC );
}
