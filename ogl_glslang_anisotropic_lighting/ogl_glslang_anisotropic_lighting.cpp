//-----------------------------------------------------------------------------
//           Name: ogl_cg_anisotropic_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: Demonstrates how to write a vertex shader using GLslang,  
//                 which calculates Anisotropic Lighting for a single light 
//                 source. The shader uses a texture as a look-up table for 
//                 the correct Anisotropic Lighting values by storing the 
//                 pre-calculated diffuse values in the texture's RGB components 
//                 and specular values in its alpha component. This style of 
//                 lighting is very useful for rendering surfaces like brushed 
//                 steel where the surface is composed of micro facets or 
//                 microscopic scratches that tend to lay parallel or run in 
//                 the same direction.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <sys/stat.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES 
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "geometry.h"
#include "tga.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

// GL_ARB_shader_objects
PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgramObjectARB  = NULL;
PFNGLDELETEOBJECTARBPROC         glDeleteObjectARB         = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgramObjectARB     = NULL;
PFNGLCREATESHADEROBJECTARBPROC   glCreateShaderObjectARB   = NULL;
PFNGLSHADERSOURCEARBPROC         glShaderSourceARB         = NULL;
PFNGLCOMPILESHADERARBPROC        glCompileShaderARB        = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC         glAttachObjectARB         = NULL;
PFNGLGETINFOLOGARBPROC           glGetInfoLogARB           = NULL;
PFNGLLINKPROGRAMARBPROC          glLinkProgramARB          = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocationARB   = NULL;
PFNGLUNIFORM4FARBPROC            glUniform4fARB            = NULL;
PFNGLUNIFORM4FVARBPROC           glUniform4fvARB           = NULL;
PFNGLUNIFORM1IARBPROC            glUniform1iARB            = NULL;
PFNGLUNIFORM1FARBPROC            glUniform1fARB            = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC     glUniformMatrix4fvARB     = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID = 0;

GLhandleARB g_programObj;
GLhandleARB g_vertexShader;
GLuint      g_location_lightVector;
GLuint      g_location_eyePosition;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void initShader(void);
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

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS",
						     "OpenGL - Anisotropic Lighting Shader Using GLslang",
                             WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, 
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
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//
    // Set up some general rendering states...
    //

	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	//
    // This is the texture used by the shader for looking up the anisotropic 
    // lighting values...
    //
	
    tgaImageFile tgaImage;
    tgaImage.load( "ogl_aniso.tga" );

    glGenTextures( 1, &g_textureID );

	glEnable( GL_TEXTURE_2D );

    glBindTexture( GL_TEXTURE_2D, g_textureID );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
		          tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, tgaImage.m_nImageData );

	//
    // It's often needed to modulate or increase the diffuse and specular 
    // terms to a small power (between 2 or 4) to account for the fact that  
    // the most-significant normal does not account for the entire lighting 
    // of the anisotropic surface.
    //

    // Turn on programmable texture combiners
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );

    glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
	glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );

    glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE );
	glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_ALPHA );

	// Modulate the texture brightness by 4X
	glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE );
    glTexEnvf( GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 4.0f );
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
// Desc: Assemble the shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
    //
    // If the required extension is present, get the addresses of its 
    // functions that we wish to use...
    //

    char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
    {
        //This extension string indicates that the OpenGL Shading Language,
        // version 1.00, is supported.
        MessageBox(NULL,"GL_ARB_shading_language_100 extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    if( strstr( ext, "GL_ARB_shader_objects" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_shader_objects extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
        glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform4fARB            = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
        glUniform4fvARB           = (PFNGLUNIFORM4FVARBPROC)wglGetProcAddress("glUniform4fvARB");
        glUniform1iARB            = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
        glUniform1fARB            = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
		glUniformMatrix4fvARB     = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");

        if( !glCreateProgramObjectARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
            !glCreateShaderObjectARB || !glCreateShaderObjectARB || !glCompileShaderARB || 
            !glGetObjectParameterivARB || !glAttachObjectARB || !glGetInfoLogARB || 
            !glLinkProgramARB || !glGetUniformLocationARB || !glUniform4fARB ||
            !glUniform4fvARB || !glUniform1iARB || !glUniform1fARB || !glUniformMatrix4fvARB )
        {
            MessageBox(NULL,"One or more GL_ARB_shader_objects functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    const char *vertexShaderStrings[1];
    GLint bVertCompiled;
    GLint bLinked;
    char str[4096];

    //
    // Create the vertex shader...
    //

    g_vertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );

    unsigned char *vertexShaderAssembly = readShaderFile( "ogl_glslang_anisotropic_lighting.vert" );
    vertexShaderStrings[0] = (char*)vertexShaderAssembly;
    glShaderSourceARB( g_vertexShader, 1, vertexShaderStrings, NULL );
    glCompileShaderARB( g_vertexShader);
    delete vertexShaderAssembly;

    glGetObjectParameterivARB( g_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, 
        &bVertCompiled );
    if( bVertCompiled  == false )
    {
        glGetInfoLogARB(g_vertexShader, sizeof(str), NULL, str);
        MessageBox( NULL, str, "Vertex Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
    }

    //
    // Create a program object and attach our anisotropic shader to it...
    //

    g_programObj = glCreateProgramObjectARB();
    glAttachObjectARB( g_programObj, g_vertexShader );

    //
    // Link the program object and print out the info log...
    //

    glLinkProgramARB( g_programObj );
    glGetObjectParameterivARB( g_programObj, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

    if( bLinked == false )
    {
        glGetInfoLogARB( g_programObj, sizeof(str), NULL, str );
        MessageBox( NULL, str, "Linking Error", MB_OK|MB_ICONEXCLAMATION );
    }

    //
    // Locate some parameters by name so we can set them later...
    //

    g_location_lightVector = glGetUniformLocationARB( g_programObj, "lightVector" );
    g_location_eyePosition = glGetUniformLocationARB( g_programObj, "eyePosition" );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void)	
{
    glDeleteTextures( 1, &g_textureID );

    glDeleteObjectARB( g_vertexShader );
    glDeleteObjectARB( g_programObj );

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
    float fLightVector[] = { 1.0f, 0.0f, 1.0f, 0.0f };
    float fEyePosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Normalize light vector
    float fLength = sqrtf( fLightVector[0]*fLightVector[0] +
    					   fLightVector[1]*fLightVector[1] +
    					   fLightVector[2]*fLightVector[2] );
    fLightVector[0] /= fLength;
    fLightVector[1] /= fLength;
    fLightVector[2] /= fLength;

    // Set the light's directional vector
    if( g_location_lightVector != -1 )
    	glUniform4fARB( g_location_lightVector, fLightVector[0], 
                        fLightVector[1], fLightVector[2], fLightVector[3] );

    // Set the viewer's eye position
    if( g_location_eyePosition != -1 )
    	glUniform4fARB( g_location_eyePosition, fEyePosition[0], 
                        fEyePosition[1], fEyePosition[2], fEyePosition[3] );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //
    // We'll start off by building a world matrix for our teapot, which will
    // let us spin it and translate it away...
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    //
    // Render a teapot...
    //

    glUseProgramObjectARB( g_programObj );

    setShaderConstants();

    renderSolidTeapot( 1.0 );
	
    glUseProgramObjectARB( NULL );

	SwapBuffers( g_hDC );
}
