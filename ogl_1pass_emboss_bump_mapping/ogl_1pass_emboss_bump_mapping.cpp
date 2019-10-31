//-----------------------------------------------------------------------------
//           Name: ogl_1pass_emboss_bump_mapping.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to perform one-pass emboss 
//                 bump mapping via OpenGL's GL_ARB_multitexture extension.
//
//   Control Keys: F1 - Toggle emboss bump mapping
//                 F2 - Toggle base texture on/off (not implemented yet)
//                 F3 - Toggle light source spinning
//                 F2 - Increase emboss factor
//                 F5 - Decrease emboss factor
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES 
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "resource.h"
#include "vector3f.h"
#include "matrix4x4f.h"
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

PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB       = NULL;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB     = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC	   g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
GLuint g_baseTextureID = -1;

float g_fSpinX          = 0.0f;
float g_fSpinY          = 0.0f;

bool  g_bEmbossBumpMap  = true;
bool  g_bMoveLightAbout = true;
bool  g_bEmbossOnly     = false;
float g_fEmbossFactor   = 1.0f;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

#define DEGTORAD(degree) ((degree) * (3.141592654f / 180.0f))

struct Vertex
{
    float tu, tv;
    float nx, ny, nz;
    float x, y, z;
};

const int NUM_VERTICES = 4;

//
// g_quadVertices will hold our regular texture-coordinates, but we'll
// store our shifted texture-coordinates in g_fEmbossTexCoords.
//

Vertex g_quadVertices[NUM_VERTICES] = 
{
//     tu    tv     nx    ny     nz     x      y     z 
    { 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, -1.0f, 0.0f },
    { 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, -1.0f, 0.0f },
    { 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f,  1.0f, 0.0f },
    { 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,   1.0f,  1.0f, 0.0f }
};

struct TexCoords
{
    float tu2, tv2;
};

TexCoords g_fEmbossTexCoords[NUM_VERTICES]; 

vector3f g_vTangents[NUM_VERTICES];
vector3f g_vBiNormals[NUM_VERTICES];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTextures(void);
void init(void);
void render(void);
void shutDown(void);

vector3f computeTangentVector(Vertex pVtxA,Vertex pVtxB,Vertex pVtxC);
void computeTangentsAndBinormals(void);
void shiftTextureCoordinates(void);
void renderQuadWithEmbossBumpMapping(void);

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
                             "OpenGL - One-Pass Emboss Bump Mapping",
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
		{
			g_dCurTime     = timeGetTime();
			g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
			g_dLastTime    = g_dCurTime;

		    render();
		}
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
		            g_bEmbossBumpMap = !g_bEmbossBumpMap;
		        break;

                case VK_F2:
                    g_bEmbossOnly = !g_bEmbossOnly;
                    if( g_bEmbossOnly == true )
                        g_bEmbossBumpMap = true;
                break;

                case VK_F3:
                    g_bMoveLightAbout = !g_bMoveLightAbout;
		        break;

                case VK_F4:
                    g_fEmbossFactor += 0.1f;
                break;

                case VK_F5:
		            g_fEmbossFactor -= 0.1f;
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
			return DefWindowProc( hWnd, msg, wParam, lParam );
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
    tgaImageFile tgaImage;
	tgaImage.load( "woodfloor.tga" );
	//tgaImage.load( "alpha_test.tga" );

    glGenTextures( 1, &g_baseTextureID );

    glBindTexture( GL_TEXTURE_2D, g_baseTextureID );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D( GL_TEXTURE_2D, 
                  0,
                  tgaImage.m_texFormat,
                  tgaImage.m_nImageWidth,
                  tgaImage.m_nImageHeight, 
                  0,
                  tgaImage.m_texFormat,
                  GL_UNSIGNED_BYTE,
                  tgaImage.m_nImageData );
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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_TEXTURE_2D );

    loadTextures();

    //
    // Set up a material, a single light source, and turn on some global 
    // ambient lighting...
    //

    glEnable( GL_LIGHTING );
    
    float ambient_matrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float diffuse_matrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_matrl );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse_matrl );

    float ambient_light0[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    float diffuse_light0[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
	float position_light0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float linearAttenuation_light0[] = { 0.3f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambient_light0 );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
    glLightfv( GL_LIGHT0, GL_LINEAR_ATTENUATION, linearAttenuation_light0 );
    glEnable( GL_LIGHT0 );

	float ambient_lightModel[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	//
	// If the required extension is present, get the addresses of its 
	// functions that we wish to use...
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
		glActiveTextureARB       = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
		glMultiTexCoord2fARB     = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");

		if( !glActiveTextureARB || !glMultiTexCoord2fARB || !glClientActiveTextureARB )
		{
			MessageBox(NULL,"One or more GL_ARB_multitexture functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	//
    // For each vertex, create a tangent vector and binormal
    //

    computeTangentsAndBinormals();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    glDeleteTextures( 1, &g_baseTextureID );

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
// Name: computeTangentVector()
// Desc: To find a tangent that heads in the direction of +tv, find
//       the components of both vectors on the tangent surface, and add a 
//       linear combination of the two projections that head in the +tv 
//       direction
//-----------------------------------------------------------------------------
vector3f computeTangentVector( Vertex pVtxA, Vertex pVtxB, Vertex pVtxC )
{
	vector3f vAB = vector3f(pVtxB.x, pVtxB.y, pVtxB.z) - vector3f(pVtxA.x, pVtxA.y, pVtxA.z);
	vector3f vAC = vector3f(pVtxC.x, pVtxC.y, pVtxC.z) - vector3f(pVtxA.x, pVtxA.y, pVtxA.z);
	vector3f nA  = vector3f(pVtxA.nx, pVtxA.ny, pVtxA.nz);

    // Components of vectors to neighboring vertices that are orthogonal to the
    // vertex normal
    vector3f vProjAB = vAB - ( dotProduct( nA, vAB ) * nA );
    vector3f vProjAC = vAC - ( dotProduct( nA, vAC ) * nA );

    // tu texture coordinate differences
    float duAB = pVtxB.tu - pVtxA.tu;
    float duAC = pVtxC.tu - pVtxA.tu;

	// tv texture coordinate differences
    float dvAB = pVtxB.tv - pVtxA.tv;
    float dvAC = pVtxC.tv - pVtxA.tv;

    if( (duAC * dvAB) > (duAB * dvAC) )
    {
        duAC = -duAC;
        duAB = -duAB;
    }
    
    vector3f vTangent = (duAC * vProjAB) - (duAB * vProjAC);
    vTangent.normalize();
    return vTangent;
}

//-----------------------------------------------------------------------------
// Name: computeTangentsAndBinormals
// Desc: For each vertex, create a tangent vector and binormal
//-----------------------------------------------------------------------------
void computeTangentsAndBinormals( void )
{	
    //
    // Even though our simple quad isn't being rendered via an index buffer. 
    // It's useful to think of our geometry as being indexed when it comes time
    // to calculate tangents and binormals. This helps to average tangent 
    // vector across all triangles that make use it, which in turn, produces  
    // much smoother results.
    // 
    // Our quad uses GL_TRIANGLE_STRIP to render, which means that our 
    // quad's four vertices will be indexed like so to create the two 
    // triangles required to create the quad:
    //
    // Tri #1 = 0, 1, 2
    // Tri #2 = 2, 3, 1
    //

    const int nNumIndices = 6;
	int indices[nNumIndices] = { 0,1,2,  2,3,1 };

    //
	// For every triangle or face, use the indices to find the vertices 
	// that make it up. Then, compute the tangent vector for each one,
	// averaging whenever a vertex happens to be shared amongst triangles.
    //

    for( int i = 0; i < nNumIndices; i += 3 )
    {
		int a = indices[i+0];
        int b = indices[i+1];
        int c = indices[i+2];

        // We use += because we want to average the tangent vectors with 
        // neighboring triangles that share vertices.
		g_vTangents[a] += computeTangentVector( g_quadVertices[a], g_quadVertices[b], g_quadVertices[c] );
	    g_vTangents[b] += computeTangentVector( g_quadVertices[b], g_quadVertices[a], g_quadVertices[c] );
	    g_vTangents[c] += computeTangentVector( g_quadVertices[c], g_quadVertices[a], g_quadVertices[b] );
	}

    //
    // Normalize each tangent vector and create a binormal to pair with it...
    //

    for( i = 0; i < NUM_VERTICES; ++i )
    {
		g_vTangents[i].normalize();

		g_vBiNormals[i] = crossProduct( vector3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz ), g_vTangents[i] );
    }
}

//-----------------------------------------------------------------------------
// Name: shiftTextureCoordinates()
// Desc: 
//-----------------------------------------------------------------------------
void shiftTextureCoordinates( void )
{
	// Get the inverse model-view matrix
	matrix4x4f modelView;
    matrix4x4f inverseModelView;
    glGetFloatv( GL_MODELVIEW_MATRIX, &modelView.m[0] );
	inverseModelView = matrix4x4f::invertMatrix( &modelView );

    // Calculate the current light position in object space
    float fLightsPosition[4];
	glGetLightfv( GL_LIGHT0, GL_POSITION, fLightsPosition );
    vector3f vLightsPosition( fLightsPosition[0], 
                              fLightsPosition[1], 
                              fLightsPosition[2] );
    inverseModelView.transformPoint( &vLightsPosition );

	//
    // Loop through the all the vertices and, based on a vector from the light 
    // to the vertex itself, calculate the correct shifted texture coordinates 
    // for emboss bump mapping.
    //

    for( int i = 0; i < NUM_VERTICES; ++i )
    {
        //
	    // Create and normalize a light vector, which points from the 
        // light's position to the current vertices' position.
	    //

        vector3f vLightToVertex;
        vLightToVertex = vLightsPosition - vector3f( g_quadVertices[i].x, g_quadVertices[i].y, g_quadVertices[i].z );
        vLightToVertex.normalize();

        float r = dotProduct( vLightToVertex, vector3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz ) );
        
        if( r < 0.0f )
        {
            // Don't shift coordinates when light is below the surface
            g_fEmbossTexCoords[i].tu2 = g_quadVertices[i].tu;
            g_fEmbossTexCoords[i].tv2 = g_quadVertices[i].tv;
        }
        else
        {
            // Shift coordinates, in tangent space, for the emboss effect.
            vector3f vEmbossShift;
            vEmbossShift.x = dotProduct( vLightToVertex, g_vTangents[i] );
            vEmbossShift.y = dotProduct( vLightToVertex, g_vBiNormals[i] );
			vEmbossShift.normalize();
            g_fEmbossTexCoords[i].tu2 = g_quadVertices[i].tu + (vEmbossShift.x / 128) * g_fEmbossFactor;
            g_fEmbossTexCoords[i].tv2 = g_quadVertices[i].tv + (vEmbossShift.y / 128) * g_fEmbossFactor;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: renderQuadWithEmbossBumpMapping()
// Desc: Perform one-pass, emboss bump mapping
//
// Note: Both Texture Stages will use the same texture, because the height 
//       map needed for emboss bump mapping is being stored in the alpha 
//       channel of the  base texture.
//
//       This means that Texture Stage 0 will work only with the RGB 
//       components of the texture, and Texture Stage 1, which does the 
//       actual bump mapping, will use the texture's alpha component to 
//       create the bumps.
//-----------------------------------------------------------------------------
void renderQuadWithEmbossBumpMapping( void )
{
	//*
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	
	//
	// Texture unit 0
	//

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_baseTextureID);
	
	// RGB: (modulate by 2X)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);

	// Alpha: (simply replace)
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);

	//
	// Texture unit 1 
	//

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_baseTextureID);
	
	// RGB:
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);// Combine mode
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);// Color operation
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);// Use result of previous texture environment as input source
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD_SIGNED_EXT);	// Alpha operation: Add Signed 2X (that's Arg0 + Arg1 - 0.5)

	// Alpha: (perform a (1-alpha) between the current texture and the result 
	//         of the previous texture environment)
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);

	glBegin( GL_TRIANGLE_STRIP );
    {
		for( int i = 0; i < NUM_VERTICES; ++i )
        {
			glNormal3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz );
			
            // Pass the regular texture coordinates for Texture Unit 0
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, g_quadVertices[i].tu, g_quadVertices[i].tv );

            // Pass the emboss shifted texture coordinates for Texture Unit 1
            glMultiTexCoord2fARB( GL_TEXTURE1_ARB, g_fEmbossTexCoords[i].tu2, g_fEmbossTexCoords[i].tv2 );
			
            glVertex3f( g_quadVertices[i].x, g_quadVertices[i].y, g_quadVertices[i].z );
		}
    }
	glEnd();

	//
    // Reset render states...
	//

	// Disable both texture units
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);

	glDisable( GL_BLEND );

	//*/

    /*

  	//
	// One-pass "Fake" Embossed Bump Mapping
	//
	// Fake emboss bump mapping is generally used with hardware renderers 
	// which are limited to two multitexture layers. It requires that the base 
	// texture and diffuse color be artificially brightened to account for the 
	// missing factor of 2 in the final modulate operation (i.e. resulting 
	// image will be darker than normal). However this can produce clamping 
	// problems on bright colors.
	//

	//
	// Texture unit 0 
	//

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_baseTextureID);

    // RGB
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_PREVIOUS_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);

    // Alpha
    glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_EXT,GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_EXT,GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_EXT,GL_SRC_ALPHA);

	//
	// Texture unit 1
	//

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_baseTextureID);

    // RGB
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_PREVIOUS_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);

    // Alpha
    glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_EXT,GL_ADD_SIGNED_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_EXT,GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_EXT,GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE1_ALPHA_EXT,GL_PREVIOUS_EXT);
    glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND1_ALPHA_EXT,GL_SRC_ALPHA);

    //
    // Render the quad...
    //

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

    glBegin( GL_TRIANGLE_STRIP );
    {
		for( int i = 0; i < NUM_VERTICES; ++i )
        {
			glNormal3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz );

            // Pass the regular texture coordinates for Texture Unit 0
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, g_quadVertices[i].tu, g_quadVertices[i].tv );

            // Pass the emboss shifted texture coordinates for Texture Unit 1
            glMultiTexCoord2fARB( GL_TEXTURE1_ARB, 
                                  g_fEmbossTexCoords[i].tu2, 
                                  g_fEmbossTexCoords[i].tv2 );
			
            glVertex3f( g_quadVertices[i].x, g_quadVertices[i].y, g_quadVertices[i].z );
		}
    }
	glEnd();

    //
    // Reset render states...
	//

	// Disable both texture units
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);

	glDisable( GL_BLEND );

	//*/
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    if( g_bMoveLightAbout == true )
    {
		//
		// Spin the light around the quad...
		//

		static float fAngle = 0; 
		fAngle += 60 * g_fElpasedTime;

		// Wrap it around, if it gets too big
		while(fAngle > 360.0f) fAngle -= 360.0f;
		while(fAngle < 0.0f)   fAngle += 360.0f;

        float x = sinf( DEGTORAD(fAngle) );
        float y = cosf( DEGTORAD(fAngle) );

        float position_light0[4];

        position_light0[0] = 2.0f * x;
        position_light0[1] = 2.0f * y;
        position_light0[2] = 0.0f;
        position_light0[3] = 1.0f;

        glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
    }

	//
    // Set up the model-view matrix to spin our quad via mouse input...
    //

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bEmbossBumpMap == true )
    {
        //
        // Shift or offset the texture coordinates used by Texture Unit 1
        //

        shiftTextureCoordinates();

		//
		// Render the quad with emboss bump mapping...
		//

		renderQuadWithEmbossBumpMapping();
    }
    else
    {
        //
        // Render the quad without any emboss bump mapping... very boring!
	    //

	    glDisable( GL_BLEND );

		glActiveTextureARB( GL_TEXTURE0_ARB );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, g_baseTextureID );

		glBegin( GL_TRIANGLE_STRIP );
        {
		    for( int i = 0; i < 4; i++ )
            {
			    glNormal3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz );
                glTexCoord2f( g_quadVertices[i].tu, g_quadVertices[i].tv );
			    glVertex3f( g_quadVertices[i].x, g_quadVertices[i].y, g_quadVertices[i].z );
		    }
        }
	    glEnd();
    }

	glPopMatrix();

	//
	// Render a small sphere to mark the light's general position...
	//
	// Note: The small sphere is simply being used as a visual cue and 
	//       doesn't represent the light's true position on the z axis.
	//       The actual light is much further away, so the embossing effect 
	//       will look sharper.
	//

	glDisable( GL_LIGHTING );

	glColor3f( 1.0f, 1.0f, 1.0f );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	float fLightsPosition[4];
	glGetLightfv( GL_LIGHT0, GL_POSITION, fLightsPosition );
	// Use x and y, but move the sphere closer in so we can see it.
	glTranslatef( fLightsPosition[0], fLightsPosition[1], -5.0f );

	renderSolidSphere( 0.05f, 8, 8 );

	glPopMatrix();

	glEnable( GL_LIGHTING );
	
	SwapBuffers( g_hDC );
}

