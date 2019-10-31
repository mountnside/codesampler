//-----------------------------------------------------------------------------
//           Name: ogl_cgfx_simple.cpp
//         Author: Kevin Harris (kevin@codesampler.com)
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to write a CgFX style shader 
//                 with OpenGL.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <CgFX/ICgFXEffect.h>
#include "vector3f.h"
#include "matrix4x4f.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID = -1;

ICgFXEffect        *g_CgFxEffect = NULL;
CgFXPARAMETER_DESC  g_CgFXparam_worldViewProj;

const int MAX_NUM_TECHNIQUES = 256;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    // GL_T2F_V3F
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 0.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);
void initCgFx(void);
void setTechniqueVariables(void);

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
						    "OpenGL - Simple CgFX Shader",WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initCgFx();

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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)	
{
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

    loadTexture();

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glEnable( GL_TEXTURE_2D );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
}

//-----------------------------------------------------------------------------
// Name: initCgFx()
// Desc: Load the CgFX effect 
//-----------------------------------------------------------------------------
void initCgFx( void )
{
    g_CgFXparam_worldViewProj.Name = 0;

    //
    // Load new effect
    //

    const char *strErrors = 0;

    if( FAILED( CgFXCreateEffectFromFileA( "ogl_cgfx_simple.fx",
                                           0, &g_CgFxEffect, &strErrors)) )
    {
        char strMessage[255];
        sprintf( strMessage, "Call to CgFXCreateEffectFromFileA failed!\n\n%s", strErrors );
        MessageBox( NULL, strMessage, "ERROR", MB_OK | MB_ICONEXCLAMATION );
        return;
    }

    // Set OpenGL device
    CgFXSetDevice( "OpenGL", NULL );

    //
    // Get the effect description and use it to retrieve the effect's 
    // parameters so they can be locally binded.
    //

    CgFXEFFECT_DESC effectDesc;
    g_CgFxEffect->GetDesc( &effectDesc );

    for( unsigned int i = 0; i < effectDesc.Parameters; ++i )
    {
        // Get the current parameter's description
        CgFXPARAMETER_DESC param;
        CGFXHANDLE paramHandle = g_CgFxEffect->GetParameter(NULL, i);
        if (FAILED(g_CgFxEffect->GetParameterDesc(paramHandle, &param)))
            continue;

        // If we're successful in retrieving a parameter, bind it to a local 
        // descriptor so we can set its value later.

        if( param.Semantic != NULL )
        {
            if (stricmp(param.Semantic, "WorldViewProjection") == 0)
                g_CgFXparam_worldViewProj = param;
        }
    }

    //
    // Our next step is to search for valid techniques. Our CgFX shader is so
    // simple that it has only one technique, but I went ahead and created a 
    // for loop suitable for querying that validity of all 256 possible 
    // techniques.
    //
    
    int numTechniques = effectDesc.Techniques;
    int currentTechnique = -1;
    bool techniqueIsValid[MAX_NUM_TECHNIQUES];

    for( int t = 0; t < numTechniques; ++t ) 
    {
        // Set technique
        if (FAILED(g_CgFxEffect->SetTechnique(g_CgFxEffect->GetTechnique(t)))) 
        {
            const char *errstr = 0;
            fprintf(stderr, "Failed to set technique: ");
            if (FAILED(CgFXGetErrors(&errstr)))
                fprintf(stderr, "unknown error\n");
            else
                fprintf(stderr, "%s", errstr);

            return;
        }

        // Validate the technique
        techniqueIsValid[t] = !FAILED(g_CgFxEffect->ValidateTechnique( g_CgFxEffect->GetCurrentTechnique() ));

        if( (currentTechnique < 0) && techniqueIsValid[t] )
            currentTechnique = t;
    }

    // Leave if there is no valid technique
    if( currentTechnique < 0 )
        return;
    else 
        g_CgFxEffect->ValidateTechnique( g_CgFxEffect->GetCurrentTechnique() );

    return;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );

    CgFXFreeDevice( "OpenGL", NULL );

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
// Name: setTechniqueVariables()
// Desc: 
//-----------------------------------------------------------------------------
void setTechniqueVariables( void )
{
	//
    // Set the projection and model-view matrices as you normally would using 
    // OpenGL's state machine.
    //


// Why does the call to gluPerspective have to be here for the sample to work?
// Normally, I set the GL_PROJECTION matrix just once prior to rendering and 
// leave it alone. CgFX doesn't seem to like this!
glMatrixMode( GL_PROJECTION );
glLoadIdentity();
gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);


    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    //
    // Once the matrix values have been set, pull the resulting matrices back 
    // out and with the help of a matrix utility class, concatenate the  
    // projection and model-view matrices together and transpose the result. 
    // This is how the CgFX shader wants them passed in.
    //

    matrix4x4f matModelView;
    matrix4x4f matProjection;
    matrix4x4f matModelViewProjection;
    matrix4x4f matModelViewProjectionTransposed;

    glGetFloatv( GL_MODELVIEW_MATRIX, &matModelView.m[0] );
    glGetFloatv( GL_PROJECTION_MATRIX, &matProjection.m[0] );

    matModelViewProjection = matProjection * matModelView;

    matModelViewProjectionTransposed = matrix4x4f::transpose( &matModelViewProjection );

    // Compute and set the world view projection matrix
    if( g_CgFXparam_worldViewProj.Name )
    {
        g_CgFxEffect->SetMatrix( g_CgFxEffect->GetParameterByName(NULL, g_CgFXparam_worldViewProj.Name),
                                 &matModelViewProjectionTransposed.m[0], 4, 4 );
    }

    g_CgFxEffect->SetTexture( "testTexture", g_textureID );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    setTechniqueVariables();

    //
    // Render the quad using the CgFX shader!
    //

    glBindTexture( GL_TEXTURE_2D, g_textureID );

    // Begin effect
    UINT nNumPasses;
    g_CgFxEffect->Begin( &nNumPasses, 0 );

    // For each pass... render the quad
    for( UINT nCurrentPass = 0; nCurrentPass < nNumPasses; ++nCurrentPass )
    {
        // Set which pass to use
        g_CgFxEffect->Pass( nCurrentPass );

        // Render our test quad
        glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
        glDrawArrays( GL_QUADS, 0, 4 );
    }

    g_CgFxEffect->End();

	SwapBuffers( g_hDC );
}
