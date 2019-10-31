//-----------------------------------------------------------------------------
//           Name: ogl_cg_geometry_program.cpp
//         Author: Kevin Harris
//  Last Modified: 08/03/07
//    Description: This sample demonstrates how to use a geometry shader to 
//                 increase the tessellation of simple triangles by subdividing 
//                 them into four new triangles.
//
//   Control Keys: F1 - Toggle usage of geometry shader
//                 F2 - Toggle wire frame mode
//
// Note: The sample shows how to write geometry shaders using both ARB assembly 
//       and Cg. Use the USE_CG_SHADERS define to pick which one to compile for.
//       The default is to use Cg.
//----------------------------------------------------------------------------- 

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

int g_nWindowWidth  = 640;
int g_nWindowHeight = 480;

GLuint g_textureID = 0;

float g_fDistance = -3.0f;
float g_fSpinX    =  0.0f;
float g_fSpinY    =  0.0f;

#define USE_CG_SHADERS
#ifdef USE_CG_SHADERS

CGprofile g_CGprofile_vertex;
CGprofile g_CGprofile_geometry;
CGcontext g_CGcontext;
CGprogram g_CGprogram_vertex;
CGprogram g_CGprogram_geometry;

#else

GLuint g_vertexProgramID;
GLuint g_geometryProgramID;

#endif

bool g_bWireFrameMode = false;
bool g_bToggleShader = true;

// GL_T2F_C4UB_V3F
struct Vertex
{
	GLfloat tu, tv;
	unsigned char r, g, b, a;
	GLfloat vx, vy, vz;
};

Vertex g_triangle[] =
{
	{ 0.0f, 0.0f, 255,   0,      0, 255, -1.0f, -0.5f, 0.0f }, // Lower-left
	{ 1.0f, 0.0f,   0, 255,      0, 255,  1.0f, -0.5f, 0.0f }, // Lower-right
	{ 0.5f, 1.0f,   0,    0,   255, 255,  0.0f,  1.0f, 0.0f }, // Top
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);
unsigned char *readShaderFile(const char *fileName);
void initShader(void);

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

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "OpenGL - Simple Cg based geometry program",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

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

    UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

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
					g_bToggleShader = !g_bToggleShader;
					break;

                case VK_F2:
                    g_bWireFrameMode = !g_bWireFrameMode;
                    break;

                case 38: // Up Arrow Key
                    g_fDistance += 1.0f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance -= 1.0f;
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
			g_nWindowWidth  = LOWORD(lParam); 
			g_nWindowHeight = HIWORD(lParam);
			glViewport(0, 0, g_nWindowWidth, g_nWindowHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)g_nWindowWidth / (GLdouble)g_nWindowHeight, 0.1, 100.0);
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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture( void )	
{
    //
	// Create a texture to test out our pixel shader...
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test.bmp" );

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
// Name: readShaderFile()
// Desc: 
//-----------------------------------------------------------------------------
unsigned char *readShaderFile( const char *fileName )
{
    FILE *file = fopen( fileName, "r" );

    if( file == NULL )
    {
        MessageBox( NULL, "Cannot open shader file!", "ERROR",
            MB_OK | MB_ICONEXCLAMATION );
		return 0;
    }

    struct _stat fileStats;

    if( _stat( fileName, &fileStats ) != 0 )
    {
        MessageBox( NULL, "Cannot get file stats for shader file!", "ERROR",
                    MB_OK | MB_ICONEXCLAMATION );
        return 0;
    }

    unsigned char *buffer = new unsigned char[fileStats.st_size];

	int bytes = fread( buffer, 1, fileStats.st_size, file );

    buffer[bytes] = 0;

	fclose( file );

	return buffer;
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the CG shader 
//-----------------------------------------------------------------------------
void initShader( void )
{

#ifdef USE_CG_SHADERS

    //
	// Create a Cg based fragment program...
    //

    if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
        g_CGprofile_vertex = CG_PROFILE_VP40;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
			        "support CG_PROFILE_VP40!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

    if( cgGLIsProfileSupported(CG_PROFILE_GPU_GP) )
        g_CGprofile_geometry = CG_PROFILE_GPU_GP;
    else
    {
        MessageBox( NULL,"Failed to initialize geometry shader! Hardware doesn't "
			        "support CG_PROFILE_GPU_GP!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

	g_CGcontext = cgCreateContext();

    const char* args[] = { "", 0 };
    //const char* args[] = { "-oglsl", 0 };

    g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext,
										          CG_SOURCE,
										          "pass_through.cg",
										          g_CGprofile_vertex,
										          NULL, 
										          NULL );

	g_CGprogram_geometry = cgCreateProgramFromFile( g_CGcontext,
										            CG_SOURCE,
										            "subdivide.cg",
										            g_CGprofile_geometry,
										            NULL, 
										            args );
    CGerror err = cgGetError(); 

    if( err )
    {
        static char str[1024];

        sprintf( str, "Error: %s\n", cgGetErrorString(err) );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );

        sprintf( str, "Error: %s\n", cgGetLastListing(g_CGcontext) );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );

	    return;
    }

	// Load the programs using Cg's expanded interface...
    cgGLLoadProgram( g_CGprogram_vertex );
	cgGLLoadProgram( g_CGprogram_geometry );

#else

    //
	// Create the vertex program...
	//

    glGenProgramsARB( 1, &g_vertexProgramID );
    glBindProgramARB( GL_VERTEX_PROGRAM_ARB, g_vertexProgramID );
    
    unsigned char *shader_assembly = readShaderFile( "pass_through.arb" );

    glProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        strlen((char*) shader_assembly), shader_assembly );
    assert( glGetError() == GL_NO_ERROR );

    delete shader_assembly;

	//
	// Create the geometry program...
	//

	// Create the geometry program
    glGenProgramsARB( 1, &g_geometryProgramID );
    glBindProgramARB( GL_GEOMETRY_PROGRAM_NV, g_geometryProgramID );
    
    shader_assembly = readShaderFile( "subdivide.arb" );

    glProgramStringARB( GL_GEOMETRY_PROGRAM_NV, GL_PROGRAM_FORMAT_ASCII_ARB,
                        strlen((char*) shader_assembly), shader_assembly );

    assert( glGetError() == GL_NO_ERROR );

    delete shader_assembly;

#endif

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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	loadTexture();

    glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );

	glEnable(GL_TEXTURE_2D);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );

    //
    // Init GLEW and verify that we got the extensions we need...
    //

    GLenum err = glewInit();

    if( err != GLEW_OK )
    {
        static char str[255];
        sprintf( str, "Error: %s\n", glewGetErrorString(err) );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    }

    if( !glewIsSupported(
        "GL_VERSION_2_0 "
		"GL_ARB_vertex_program "
        "GL_ARB_fragment_program "
        "GL_EXT_texture_array "
        "GL_NV_gpu_program4 "   // Also initializes NV_fragment_program4 etc.
        ))
    {
        static char str[1024];
        sprintf( str, "Unable to load extension(s), this sample requires:\n"
                        "  OpenGL version 2.0\n"
                        "  GL_ARB_vertex_program\n"
                        "  GL_ARB_fragment_program\n"
                        "  GL_EXT_texture_array\n"
                        "  GL_NV_gpu_program4\n"
                        "Exiting...\n" );

        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION ); 

        exit(-1);
    }
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );

#ifdef USE_CG_SHADERS

    cgDestroyProgram( g_CGprogram_vertex );
	cgDestroyProgram( g_CGprogram_geometry );
	cgDestroyContext( g_CGcontext );

#else

    glDeleteProgramsARB( 1, &g_vertexProgramID );
	glDeleteProgramsARB( 1, &g_geometryProgramID );

#endif

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
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, g_fDistance );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bWireFrameMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    if( g_bToggleShader == true )
    {
#ifdef USE_CG_SHADERS
		cgGLBindProgram( g_CGprogram_vertex );
		cgGLEnableProfile( g_CGprofile_vertex );

		cgGLBindProgram( g_CGprogram_geometry );
		cgGLEnableProfile( g_CGprofile_geometry );
#else
        glEnable( GL_VERTEX_PROGRAM_ARB );
	    glBindProgramARB( GL_VERTEX_PROGRAM_ARB, g_vertexProgramID );
    	
        glEnable( GL_GEOMETRY_PROGRAM_NV );
	    glBindProgramARB( GL_GEOMETRY_PROGRAM_NV, g_geometryProgramID );
#endif
    }
        glBindTexture( GL_TEXTURE_2D, g_textureID );
        glInterleavedArrays( GL_T2F_C4UB_V3F, 0, g_triangle );
        glDrawArrays( GL_TRIANGLES, 0, 3 );

#ifdef USE_CG_SHADERS
        cgGLDisableProfile( g_CGprofile_geometry );
		cgGLDisableProfile( g_CGprofile_vertex );
#else
        glDisable( GL_GEOMETRY_PROGRAM_NV );
	    glDisable( GL_VERTEX_PROGRAM_ARB );
#endif

	SwapBuffers( g_hDC );
}
