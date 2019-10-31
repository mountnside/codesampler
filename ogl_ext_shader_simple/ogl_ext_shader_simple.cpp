//-----------------------------------------------------------------------------
//           Name: ogl_ext_shader_simple.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to write a vertex shader using 
//                 ATI's EXT_vertex_shader extension.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------
#include "glati.h"

PFNGLBINDPARAMETEREXTPROC     glBindParameterEXT     = NULL;
PFNGLGENVERTEXSHADERSEXTPROC  glGenVertexShadersEXT  = NULL;
PFNGLBINDVERTEXSHADEREXTPROC  glBindVertexShaderEXT  = NULL;
PFNGLBEGINVERTEXSHADEREXTPROC glBeginVertexShaderEXT = NULL;
PFNGLGENSYMBOLSEXTPROC        glGenSymbolsEXT        = NULL;
PFNGLSHADEROP2EXTPROC         glShaderOp2EXT         = NULL;
PFNGLSHADEROP1EXTPROC         glShaderOp1EXT         = NULL;
PFNGLENDVERTEXSHADEREXTPROC   glEndVertexShaderEXT   = NULL;
PFNGLSETINVARIANTEXTPROC      glSetInvariantEXT      = NULL;
PFNGLSETLOCALCONSTANTEXTPROC  glSetLocalConstantEXT  = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_vertexProgramID;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    unsigned char r, g, b, a;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
//     r    g    b    a      x     y     z
    { 255, 255,   0, 255, -1.0f,-1.0f, 0.0f, }, // Bottom-Left,  color = yellow
    { 255,   0,   0, 255,  1.0f,-1.0f, 0.0f, }, // Bottom-Right, color = red
    {   0, 255,   0, 255,  1.0f, 1.0f, 0.0f, }, // Top-Right,    color = green
    {   0,   0, 255, 255, -1.0f, 1.0f, 0.0f  }, // Top-Left,     color = blue
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Simple Vertex Shader Using EXT_vertex_shader",
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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
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

	if( strstr( ext, "GL_EXT_vertex_shader" ) == NULL )
	{
		MessageBox(NULL,"GL_EXT_vertex_shader extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		glBindParameterEXT     = (PFNGLBINDPARAMETEREXTPROC)wglGetProcAddress("glBindParameterEXT");
		glGenVertexShadersEXT  = (PFNGLGENVERTEXSHADERSEXTPROC)wglGetProcAddress("glGenVertexShadersEXT");
		glBindVertexShaderEXT  = (PFNGLBINDVERTEXSHADEREXTPROC)wglGetProcAddress("glBindVertexShaderEXT");
		glBeginVertexShaderEXT = (PFNGLBEGINVERTEXSHADEREXTPROC)wglGetProcAddress("glBeginVertexShaderEXT");
		glGenSymbolsEXT        = (PFNGLGENSYMBOLSEXTPROC)wglGetProcAddress("glGenSymbolsEXT");
		glShaderOp2EXT         = (PFNGLSHADEROP2EXTPROC)wglGetProcAddress("glShaderOp2EXT");
		glShaderOp1EXT         = (PFNGLSHADEROP1EXTPROC)wglGetProcAddress("glShaderOp1EXT");
		glEndVertexShaderEXT   = (PFNGLENDVERTEXSHADEREXTPROC)wglGetProcAddress("glEndVertexShaderEXT");
		glSetInvariantEXT      = (PFNGLSETINVARIANTEXTPROC)wglGetProcAddress("glSetInvariantEXT");
		glSetLocalConstantEXT  = (PFNGLSETLOCALCONSTANTEXTPROC)wglGetProcAddress("glSetLocalConstantEXT");

		if( !glBindParameterEXT || !glGenVertexShadersEXT || !glBindVertexShaderEXT || 
	        !glBeginVertexShaderEXT || !glGenSymbolsEXT || !glShaderOp2EXT ||
			!glShaderOp1EXT || !glEndVertexShaderEXT || !glSetInvariantEXT )
		{
			MessageBox(NULL,"One or more GL_EXT_vertex_shader functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	//
	// Create and bind our vertex shader
	//

	g_vertexProgramID = glGenVertexShadersEXT( 1 );
	glBindVertexShaderEXT( g_vertexProgramID );

	//
	// Bind OpenGL state data, which will be required by our shader
	//

	unsigned int modelView  = glBindParameterEXT( GL_MODELVIEW_MATRIX );
	unsigned int projection = glBindParameterEXT( GL_PROJECTION_MATRIX );
	unsigned int vertex     = glBindParameterEXT( GL_CURRENT_VERTEX_EXT );
	unsigned int color      = glBindParameterEXT( GL_CURRENT_COLOR );
	
	//
	// Our shader will simply transform the vertex and output the color passed in.
	//

	unsigned int vertexInEyeSpace;
	unsigned int constantColor;
	float constantColorValue[4] = { 0.0f, 1.0f, 0.0f, 1.0f}; // Green

	glBeginVertexShaderEXT();
	{
		// Create a constant
		constantColor = glGenSymbolsEXT( GL_VECTOR_EXT, GL_LOCAL_CONSTANT_EXT, GL_FULL_RANGE_EXT, 1 );
		
		// Set the value of our constant.
		glSetLocalConstantEXT( constantColor, GL_FLOAT, constantColorValue );

		// Create a local value.
		vertexInEyeSpace = glGenSymbolsEXT( GL_VECTOR_EXT, GL_LOCAL_EXT, GL_FULL_RANGE_EXT, 1 );

		// Transform the vertex by the model-view matrix.
		glShaderOp2EXT( GL_OP_MULTIPLY_MATRIX_EXT, vertexInEyeSpace, modelView, vertex );

		// Transform the vertex, which is now in eye-space, by the projection
		// matrix, and output it.
		glShaderOp2EXT( GL_OP_MULTIPLY_MATRIX_EXT, GL_OUTPUT_VERTEX_EXT, projection, vertexInEyeSpace );

		// Output the color.
		glShaderOp1EXT( GL_OP_MOV_EXT, GL_OUTPUT_COLOR0_EXT, color );

		// Uncomment this to use the constant color instead of the color passed.
		//glShaderOp1EXT( GL_OP_MOV_EXT, GL_OUTPUT_COLOR0_EXT, constantColor );
	}
	glEndVertexShaderEXT();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
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
	
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	glEnable( GL_VERTEX_SHADER_EXT );
	glBindVertexShaderEXT( g_vertexProgramID );

	glInterleavedArrays( GL_C4UB_V3F, 0, g_quadVertices );
	glDrawArrays( GL_QUADS, 0, 4 );

	glDisable( GL_VERTEX_SHADER_EXT );

	SwapBuffers( g_hDC );
}
