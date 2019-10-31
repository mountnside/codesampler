//-----------------------------------------------------------------------------
//           Name: ogl_cg_volumetric_lines.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to render fake volumetric 
//                 lines Using Cg. The technique is ideal for rendering effects 
//                 such as laser fire, tracer rounds, and neon signs.
//
//                 The sample is based on the "Cg Volume Lines" sample, which 
//                 ships with the nVIDIA SDK v8.0. See the whitepaper titled, 
//                 "VolumeLine.pdf" in the SDK for more information.
//
//   Control Keys: v - Increase volume width.
//                 V - Decrease volume width.
//                 s - Toggle usage of shader.
//                 t - Change texture.
//                 w - Toggle wire-frame mode. (Only works well with "debug_numbers.bmp")
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>

#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"

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

// GL_ARB_multitexture
PFNGLACTIVETEXTUREPROC       glActiveTexture       = NULL;
PFNGLMULTITEXCOORD2FPROC     glMultiTexCoord2f     = NULL;
PFNGLMULTITEXCOORD3FPROC     glMultiTexCoord3f     = NULL;
PFNGLMULTITEXCOORD4FPROC     glMultiTexCoord4f     = NULL;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;

const int g_nNumTextures = 4;
GLuint g_volumeLineTextureIDs[g_nNumTextures];
int g_nCurrentTexture = 0;

float g_fVolumeWidth = 0.2f;
bool g_bUseShaders = true;
bool g_bRenderInWireFrame = false;

CGprofile g_CGprofile_vertex;
CGprofile g_CGprofile_fragment;
CGcontext g_CGcontext;
CGprogram g_CGprogram_vertex;
CGprogram g_CGprogram_fragment;

CGparameter g_CGparam_modelViewProj;
CGparameter g_CGparam_modelView;
CGparameter g_CGparam_volumeLineTexture;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

// GL_C3F_V3F
struct Vertex
{
    float r, g, b;
    float x, y, z;
};

Vertex g_lineVertices[] =
{
//     r     g     b       x     y     z
    { 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f },

    { 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f,   1.0f,-1.0f, 0.0f },

    { 1.0f, 1.0f, 1.0f,   1.0f,-1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 0.0f },

    { 1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
GLuint loadTexture(string* fileName);
void init(void);
void render(void);
void shutDown(void);
void initShader(void);
void renderVolumeLine(float fFromPos_x, float fFromPos_y, float fFromPos_z,
					  float fToPos_x, float fToPos_y, float fToPos_z,
					  float fVolumeWidth );

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
						     "OpenGL - Rendering Fake Volumetric Lines With Cg",
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
                case 'v':
					g_fVolumeWidth += 0.1f;
					break;

				case 'V':
					g_fVolumeWidth -= 0.1f;
					break;

                case 's':
					g_bUseShaders = !g_bUseShaders;
					break;

                case 't':
					++g_nCurrentTexture;
                    if( g_nCurrentTexture >= g_nNumTextures )
                        g_nCurrentTexture = 0;
					break;

				case 'w':
					g_bRenderInWireFrame = !g_bRenderInWireFrame;
					if( g_bRenderInWireFrame == true )
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
					else
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
GLuint loadTexture(string* fileName)
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( fileName->c_str() );

    GLuint textureID;

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &textureID );

		glBindTexture( GL_TEXTURE_2D, textureID );

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

    return textureID;
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

    string fileName;
    fileName.assign(".\\blue_glow.bmp");
    g_volumeLineTextureIDs[0] = loadTexture( &fileName );

    fileName.assign(".\\red_laser.bmp");
    g_volumeLineTextureIDs[1] = loadTexture( &fileName );

    fileName.assign(".\\fire_spiral.bmp");
    g_volumeLineTextureIDs[2] = loadTexture( &fileName );

    fileName.assign(".\\debug_numbers.bmp");
    g_volumeLineTextureIDs[3] = loadTexture( &fileName );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

    //
    // GL_ARB_multitexture
    //

    char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_multitexture" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_multitexture extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glActiveTexture       = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
        glMultiTexCoord2f     = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
        glMultiTexCoord3f     = (PFNGLMULTITEXCOORD3FPROC)wglGetProcAddress("glMultiTexCoord3f");
        glMultiTexCoord4f     = (PFNGLMULTITEXCOORD4FPROC)wglGetProcAddress("glMultiTexCoord4f");
        glClientActiveTexture = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTexture");

        if( !glActiveTexture || !glMultiTexCoord2f || !glMultiTexCoord3f ||
            !glMultiTexCoord4f || !glClientActiveTexture )
        {
            MessageBox(NULL,"One or more GL_ARB_multitexture functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
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
    // Search for a valid vertex shader profile in this order:
    //
    // CG_PROFILE_ARBVP1 - GL_ARB_vertex_program
    // CG_PROFILE_VP40   - GL_ARB_vertex_program + GL_NV_vertex_program3
    //

    if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        g_CGprofile_vertex = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
        g_CGprofile_vertex = CG_PROFILE_VP40;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
            "support any of the required vertex shading extensions!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return;
    }

	//
	// Search for a valid fragment shader profile in this order:
	//
	// CG_PROFILE_ARBFP1 - GL_ARB_fragment_program
	// CG_PROFILE_FP30   - GL_NV_fragment_program
	// CG_PROFILE_FP20   - NV_texture_shader & NV_register_combiners
	//
	
	if( cgGLIsProfileSupported(CG_PROFILE_ARBFP1) )
        g_CGprofile_fragment = CG_PROFILE_ARBFP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_FP30) )
        g_CGprofile_fragment = CG_PROFILE_FP30;
	else if( cgGLIsProfileSupported(CG_PROFILE_FP20) )
        g_CGprofile_fragment = CG_PROFILE_FP20;
    else
    {
        MessageBox( NULL,"Failed to initialize fragment shader! Hardware doesn't "
			        "support any of the required fragment shading extensions!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create the vertex and fragment shader...
	//
	
	g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext, CG_SOURCE,
										          "volumeLines_vertex.cg",
										          g_CGprofile_vertex,
										          NULL, NULL );

	g_CGprogram_fragment = cgCreateProgramFromFile( g_CGcontext, CG_SOURCE,
										            "volumeLines_fragment.cg",
										            g_CGprofile_fragment,
										            NULL, NULL );

	//
	// Load the programs using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram_vertex );
	cgGLLoadProgram( g_CGprogram_fragment );

	//
	// Bind some parameters by name so we can set them later...
	//

	// uniform parameters
	g_CGparam_modelViewProj = cgGetNamedParameter( g_CGprogram_vertex, "ModelViewProj" );
	g_CGparam_modelView = cgGetNamedParameter( g_CGprogram_vertex, "ModelView" );
	g_CGparam_volumeLineTexture = cgGetNamedParameter( g_CGprogram_fragment, "volumeLineTexure" );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    for( int i = 0; i < g_nNumTextures; ++i )
        glDeleteTextures( 1, &g_volumeLineTextureIDs[i] );

    cgDestroyProgram( g_CGprogram_vertex );
	cgDestroyProgram( g_CGprogram_fragment );
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
// Name: renderVolumeLine()
// Desc: 
//-----------------------------------------------------------------------------
void renderVolumeLine( float fFromPos_x, float fFromPos_y, float fFromPos_z,
					   float fToPos_x,   float fToPos_y,   float fToPos_z,
					   float fVolumeWidth )
{
	//
	// Each volume line is rendered as a quad, so we need to make sure that 
	// GL_QUADS is the currently selected primitive.
	//

	float fHalfVolumeWidth = fVolumeWidth * 0.5f;

    glMultiTexCoord3f( GL_TEXTURE0, 0.0f, 0.0f, fVolumeWidth );           // tu, tv, width
    glMultiTexCoord2f( GL_TEXTURE1, -fVolumeWidth, fHalfVolumeWidth );    // width tweaks
    glMultiTexCoord3f( GL_TEXTURE2, fToPos_x, fToPos_y, fToPos_z );       // end position
    glVertex3f( fFromPos_x, fFromPos_y, fFromPos_z );                     // vertex 0 of quad

    glMultiTexCoord3f( GL_TEXTURE0, 0.25f, 0.0f, fVolumeWidth );          // tu, tv, width
    glMultiTexCoord2f( GL_TEXTURE1, fVolumeWidth, fHalfVolumeWidth );     // width tweaks
    glMultiTexCoord3f( GL_TEXTURE2, fFromPos_x, fFromPos_y, fFromPos_z ); // end position
    glVertex3f( fToPos_x, fToPos_y, fToPos_z );                           // vertex 1 of quad

	glMultiTexCoord3f( GL_TEXTURE0, 0.25f, 0.25f, fVolumeWidth );         // tu, tv, width
	glMultiTexCoord2f( GL_TEXTURE1, -fVolumeWidth, fHalfVolumeWidth );    // width tweaks
    glMultiTexCoord3f( GL_TEXTURE2, fFromPos_x, fFromPos_y, fFromPos_z ); // end position
    glVertex3f( fToPos_x, fToPos_y, fToPos_z );                           // vertex 2 of quad

	glMultiTexCoord3f( GL_TEXTURE0, 0.0f, 0.25f, fVolumeWidth );          // tu, tv, width
	glMultiTexCoord2f( GL_TEXTURE1, fVolumeWidth, fHalfVolumeWidth );     // width tweaks
    glMultiTexCoord3f( GL_TEXTURE2, fToPos_x, fToPos_y, fToPos_z );       // end position
    glVertex3f( fFromPos_x, fFromPos_y, fFromPos_z );                     // vertex 3 of quad
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

    if( g_bUseShaders )
    {
        //
        // Render our line list as thin quads and have the fake volumetric 
        // line shader apply special textures to them as their angle to the
        // viewer changes.
        //

        glDisable( GL_LIGHTING );
	    glDisable( GL_CULL_FACE );

	    glEnable( GL_BLEND );
	    glBlendFunc( GL_ONE, GL_ONE );

	    glDepthMask( GL_FALSE );
	    glEnable( GL_DEPTH_TEST );

	    cgGLBindProgram( g_CGprogram_vertex );
	    cgGLEnableProfile( g_CGprofile_vertex );

	    cgGLBindProgram( g_CGprogram_fragment );
	    cgGLEnableProfile( g_CGprofile_fragment );

	    cgGLSetStateMatrixParameter( g_CGparam_modelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY );
	    cgGLSetStateMatrixParameter( g_CGparam_modelView, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY );

	    cgGLEnableTextureParameter( g_CGparam_volumeLineTexture );
	    cgGLSetTextureParameter( g_CGparam_volumeLineTexture, g_volumeLineTextureIDs[g_nCurrentTexture] );

	    //
	    // Each line in our line list will get rendered as a quad, so we need to 
        // make sure that GL_QUADS is the currently selected primitive before 
        // calling renderVolumeLine.
	    //

	    glBegin( GL_QUADS );
	    {
		    renderVolumeLine( g_lineVertices[0].x, g_lineVertices[0].y, g_lineVertices[0].z, // Position 1 of line segment
						      g_lineVertices[1].x, g_lineVertices[1].y, g_lineVertices[1].z, // Position 2 of line segment
						      g_fVolumeWidth );                                              // width of line segment

		    renderVolumeLine( g_lineVertices[2].x, g_lineVertices[2].y, g_lineVertices[2].z,
						      g_lineVertices[3].x, g_lineVertices[3].y, g_lineVertices[3].z,
						      g_fVolumeWidth );

		    renderVolumeLine( g_lineVertices[4].x, g_lineVertices[4].y, g_lineVertices[4].z,
						      g_lineVertices[5].x, g_lineVertices[5].y, g_lineVertices[5].z,
						      g_fVolumeWidth );

		    renderVolumeLine( g_lineVertices[6].x, g_lineVertices[6].y, g_lineVertices[6].z,
						      g_lineVertices[7].x, g_lineVertices[7].y, g_lineVertices[7].z,
						      g_fVolumeWidth );
	    }
	    glEnd();

	    cgGLDisableTextureParameter( g_CGparam_volumeLineTexture );

	    cgGLDisableProfile( g_CGprofile_vertex );
	    cgGLDisableProfile( g_CGprofile_fragment );
    }
    else
    {
        //
        // Just render our line list...
        //

        glDisable( GL_TEXTURE_2D );
        glDisable( GL_LIGHTING );
        glDisable( GL_BLEND );

        glInterleavedArrays( GL_C3F_V3F, 0, g_lineVertices );
		glDrawArrays( GL_LINES, 0, 8 );
    }

	SwapBuffers( g_hDC );
}

