//-----------------------------------------------------------------------------
//           Name: ogl_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates the three basic types of lights
//                 that are available in OpenGL: directional, spot, and point. 
//
//   Control Keys: F1 - Changes the light's type
//                 F2 - Toggles wire frame mode
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES 
#include <math.h>
#include <iostream>
using namespace std;
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

bool g_bRenderInWireFrame = false;

enum LightTypes
{
    LIGHT_TYPE_DIRECTIONAL = 0,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_POINT,
};

int g_lightType = LIGHT_TYPE_DIRECTIONAL;

// Mesh properties...
const int   g_nNumVertsAlongX   = 32;
const int   g_nNumVertsAlongZ   = 32;
const float g_fMeshLengthAlongX = 10.0f;
const float g_fMeshLengthAlongZ = 10.0f;

const int g_nMeshVertCount = (g_nNumVertsAlongX-1) * (g_nNumVertsAlongZ-1) * 6;

struct Vertex
{
	// GL_C4F_N3F_V3F
	float r, g, b, a;
	float nx, ny, nz;
	float x, y, z;
};

Vertex g_meshVertices[g_nMeshVertCount];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void initLights(void);
void createMesh(void);
void invertMatrix( const float *m, float *out );

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
                             "OpenGL - Lighting",
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
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
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

				case VK_F1:
		            ++g_lightType;
		            if(g_lightType > 2)
			            g_lightType = 0;
		        break;

                case VK_F2:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        glPolygonMode( GL_FRONT, GL_LINE );
                    else
                        glPolygonMode( GL_FRONT, GL_FILL );
                break;
			}
		}
        break;

        case WM_SIZE:
		{
            int nWidth  = LOWORD(lParam);
            int nHeight = HIWORD(lParam);

			glViewport( 0, 0, (GLsizei)nWidth, (GLsizei)nHeight );
            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
            glMatrixMode( GL_MODELVIEW );
		}
        break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
        break;

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

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	initLights();
	createMesh();
}

//-----------------------------------------------------------------------------
// Name: initLights()
// Desc: 
//-----------------------------------------------------------------------------
void initLights( void )
{
	GLfloat mat_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv( GL_FRONT, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient );   
 
	// Set light 0 to be a simple, bright directional light to use 
    // on the mesh that will represent light 2
	GLfloat diffuse_light0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat position_light0[] = { 0.5f, -0.5f, -0.5f, 0.0f };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
	glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

	// Set light 1 to be a simple, faint grey directional light so 
    // the walls and floor are slightly different shades of grey
	GLfloat diffuse_light1[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	GLfloat position_light1[] = { 0.3f, -0.5f, 0.2f, 0.0f };
	//GLfloat position_light1[] = { 0.3f, -0.5f, -0.2f, 0.0f };
	glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse_light1 );
	glLightfv( GL_LIGHT1, GL_POSITION, position_light1 );

	// Light #2 will be the demo light used to light the floor and walls. 
	// It will be set up in render() since its type can be changed at 
    // run-time.

	// Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
	GLfloat ambient_lightModel[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );
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
			++i;

			// 1
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			++i;

			// 2
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			++i;

			// Second triangle of the current quad
			// 2 ___ 1
			//  |  /|
			//  |/__|
			// 0

			// 0
			g_meshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			++i;

			// 1
			g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			++i;

			// 2
			g_meshVertices[i].x = g_fMeshLengthAlongX * x * dX;
			g_meshVertices[i].z = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			++i;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void)	
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
// Name: invertMatrix()
// Desc: Performs a highly general un-optimized inversion of a 4x4 matrix.
//-----------------------------------------------------------------------------
void invertMatrix( const float *m, float *out )
{
	// OpenGL matrices are column major and can be quite confusing to access 
	// when stored in the typical, one-dimensional array often used by the API.
	// Here are some shorthand conversion macros, which convert a row/column 
	// combination into an array index.
	
	#define MAT(m,r,c) m[c*4+r]

	#define m11 MAT(m,0,0)
	#define m12 MAT(m,0,1)
	#define m13 MAT(m,0,2)
	#define m14 MAT(m,0,3)
	#define m21 MAT(m,1,0)
	#define m22 MAT(m,1,1)
	#define m23 MAT(m,1,2)
	#define m24 MAT(m,1,3)
	#define m31 MAT(m,2,0)
	#define m32 MAT(m,2,1)
	#define m33 MAT(m,2,2)
	#define m34 MAT(m,2,3)
	#define m41 MAT(m,3,0)
	#define m42 MAT(m,3,1)
	#define m43 MAT(m,3,2)
	#define m44 MAT(m,3,3)

	// Inverse = adjoint / det. (See linear algebra texts.)

	// pre-compute 2x2 dets for last two rows when computing
	// cofactors of first two rows.
	float d12 = (m31 * m42 - m41 * m32);
	float d13 = (m31 * m43 - m41 * m33);
	float d23 = (m32 * m43 - m42 * m33);
	float d24 = (m32 * m44 - m42 * m34);
	float d34 = (m33 * m44 - m43 * m34);
	float d41 = (m34 * m41 - m44 * m31);

	float tmp[16];
	
	tmp[0] =  (m22 * d34 - m23 * d24 + m24 * d23);
	tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
	tmp[2] =  (m21 * d24 + m22 * d41 + m24 * d12);
	tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

	// Compute determinant as early as possible using these cofactors.
	float det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];

	// Run singularity test.
	if( det == 0.0 )
	{
		cout << "Warning: Call to invertMatrix produced a Singular matrix." << endl;

		float identity[16] = 
		{
		   1.0, 0.0, 0.0, 0.0,
		   0.0, 1.0, 0.0, 0.0,
		   0.0, 0.0, 1.0, 0.0,
		   0.0, 0.0, 0.0, 1.0
		};

	   memcpy( out, identity, 16*sizeof(float) );
	}
	else 
	{
	   float invDet = 1.0f / det;
	   
	   // Compute rest of inverse.
	   tmp[0] *= invDet;
	   tmp[1] *= invDet;
	   tmp[2] *= invDet;
	   tmp[3] *= invDet;

	   tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
	   tmp[5] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
	   tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
	   tmp[7] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

	   // Pre-compute 2x2 dets for first two rows when computing cofactors 
	   // of last two rows.
	   d12 = m11 * m22 - m21 * m12;
	   d13 = m11 * m23 - m21 * m13;
	   d23 = m12 * m23 - m22 * m13;
	   d24 = m12 * m24 - m22 * m14;
	   d34 = m13 * m24 - m23 * m14;
	   d41 = m14 * m21 - m24 * m11;

	   tmp[8]  =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
	   tmp[9]  = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
	   tmp[10] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
	   tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
	   tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
	   tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
	   tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
	   tmp[15] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;

	   memcpy( out, tmp, 16*sizeof(float) );
	}

	#undef m11
	#undef m12
	#undef m13
	#undef m14
	#undef m21
	#undef m22
	#undef m23
	#undef m24
	#undef m31
	#undef m32
	#undef m33
	#undef m34
	#undef m41
	#undef m42
	#undef m43
	#undef m44
	#undef MAT
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render(void)	
{
	//
	// Create light 2 at run-time based on the light type the user has
    // selected.
	//

	static double dStartTime = timeGetTime();
	float fElpasedAppTime = (float)((timeGetTime() - dStartTime) * 0.001);

	float x =   sinf( fElpasedAppTime * 2.000f );
    float y =   sinf( fElpasedAppTime * 2.246f );
    float z = -(sinf( fElpasedAppTime * 2.640f ));

	// Since we want to reuse GL_LIGHT2 to demonstrate the differnt types of 
    // lights at run-time, we'll push the original or default lighting 
    // attributes of the OpenGL state machine before we start modifying 
    // anything. This way, we can pop and restore the defualt setup before we 
    // setup a new light type.
	
    glPushAttrib( GL_LIGHTING_BIT );

	//
	// While both Direct3D and OpenGL use the same formula for lighting 
	// attenuation, they call the variables by different names when setting 
	// them through the API. The following two formulas are the same and 
	// only differ by the API names used for each variable.
	//
	// Direct3D:
	//
	// attenuation = 1 / ( Attenuation0 +
	//                     Attenuation1 * d +
	//                     Attenuation2 * d2 )
	//
	// OpenGL:
	//
	// attenuation = 1 / ( GL_CONSTANT_ATTENUATION  +
	//                     GL_LINEAR_ATTENUATION    * d +
	//                     GL_QUADRATIC_ATTENUATION * d2 )
	//
	// Where:  d = Distance from vertex position to light position
	//        d2 = d squared
	//

	//
	// You should note that GL_POSITION is used for both spot lights and 
	// directional lights in OpenGL.
	// 
	// If the w component of the position is 0.0, the light is treated 
	// as a directional source and x, y, and z represent a direction vector.
	//
	// If the w component of the position is 1.0, the light is treated 
	// as a positionable light source and x, y, and z represent the lights 
	// position in eye coordinates as the light's position will be transformed 
	// by the modelview matrix when glLight is called.
	//

    switch( g_lightType )
    {
        case LIGHT_TYPE_DIRECTIONAL:
		{
			GLfloat diffuse_light2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			GLfloat position_light2[] = { x, y, z, 0.0f };
			glLightfv( GL_LIGHT2, GL_DIFFUSE, diffuse_light2 );
			glLightfv( GL_LIGHT2, GL_POSITION, position_light2 );
		}
        break;

        case LIGHT_TYPE_SPOT:
		{
			GLfloat diffuse_light2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			GLfloat position_light2[] = { 2.0f*x, 2.0f*y, 2.0f*z, 1.0f };
			GLfloat spotDirection_light2[] = { x, y, z };
			GLfloat constantAttenuation_light2[] = { 1.0f };
			glLightfv( GL_LIGHT2, GL_DIFFUSE, diffuse_light2 );
			glLightfv( GL_LIGHT2, GL_POSITION, position_light2 );
			glLightfv( GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection_light2 );
			glLightfv( GL_LIGHT2, GL_CONSTANT_ATTENUATION, constantAttenuation_light2 );
			glLightf( GL_LIGHT2, GL_SPOT_CUTOFF, 45.0f );
			glLightf( GL_LIGHT2, GL_SPOT_EXPONENT, 25.0f );
		}
        break;

        case LIGHT_TYPE_POINT:
		{
			GLfloat diffuse_light2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			GLfloat position_light2[] = { 4.5f*x, 4.5f*y, 4.5f*z, 1.0f };
			GLfloat linearAttenuation_light2[] = { 0.4f };
			glLightfv( GL_LIGHT2, GL_DIFFUSE, diffuse_light2 );
			glLightfv( GL_LIGHT2, GL_POSITION, position_light2 );
			glLightfv( GL_LIGHT2, GL_LINEAR_ATTENUATION , linearAttenuation_light2 );
		}
        break;
    }

	//
	// Render...
	//

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	gluLookAt(-12.0, 12.0, 12.0,  // Camera position
               0.0,-1.2, 0.0,     // Look-at point
               0.0, 1.0, 0.0 );   // Up vector

	// The first thing we draw is our walls. The walls are to be lit by 
	// lights 1 and 2 only, so we need to turn on lights 1 and 2, and turn off 
	// light 0. Light 0 will be used later for the 3D primitives.

    glDisable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    glEnable( GL_LIGHT2 );

	// Draw the floor
	glPushMatrix();
	glTranslatef( -5.0f, -5.0f, 5.0f );
	glInterleavedArrays( GL_C4F_N3F_V3F, 0, g_meshVertices );
    glDrawArrays( GL_TRIANGLES , 0, g_nMeshVertCount );
	glPopMatrix();

	// Draw the back wall
	glPushMatrix();
	glTranslatef( 5.0f, -5.0f, 5.0f );
	glRotatef( 90.0f, 0.0f, 0.0f, 1.0f );
	glInterleavedArrays( GL_C4F_N3F_V3F, 0, g_meshVertices );
    glDrawArrays( GL_TRIANGLES , 0, g_nMeshVertCount );
	glPopMatrix();

	// Draw the side wall
	glPushMatrix();
	glTranslatef( -5.0f, -5.0f, -5.0f );
	glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
	glInterleavedArrays( GL_C4F_N3F_V3F, 0, g_meshVertices );
    glDrawArrays( GL_TRIANGLES , 0, g_nMeshVertCount );
	glPopMatrix();

    // We're finshed drawing the walls, we'll now draw a simple 
	// 3D primitive to represent the light's type. We'll use a little cone 
	// for a directional or spot light and a little sphere for a point light.
    // Light 0 is just for our primitives, so turn on light 0, and 
    // turn off lights 1 and 2 before rendering.

    glEnable( GL_LIGHT0 );
    glDisable( GL_LIGHT1 );
    glDisable( GL_LIGHT2 );

	// Draw the correct 3d primitve representing the current light type...
    if( g_lightType == LIGHT_TYPE_DIRECTIONAL )
	{
		glPushMatrix();

		GLfloat position_light2[4];
		glGetLightfv( GL_LIGHT2, GL_POSITION, position_light2 );

		gluLookAt( 0.0f, 0.0f, 0.25f , // Light's position (add a 0.25f offset to center our light's cone)
				   0.0f + position_light2[0], 0.0f + position_light2[1], position_light2[2],
                   0.0, 1.0, 0.0 );    // Up vector

		renderSolidCone( 0.2f, 0.6f, 15, 15 );

		glPopMatrix();
	}
    else if( g_lightType == LIGHT_TYPE_SPOT )
    {
        glPushMatrix();
		glLoadIdentity();

		GLfloat position_light2[4];
        GLfloat spotDirection_light2[4];
		glGetLightfv( GL_LIGHT2, GL_POSITION, position_light2 );
		glGetLightfv( GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection_light2 );

		gluLookAt( position_light2[0], position_light2[1], position_light2[2], // Light's position
                   position_light2[0] + spotDirection_light2[0], // Look-at point
                   position_light2[1] + spotDirection_light2[1],
                   position_light2[2] + spotDirection_light2[2], 
                   0.0, 1.0, 0.0 );                              // Up vector

		GLfloat modelViewMat[16];
		GLfloat invModelViewMat[16];
		glGetFloatv( GL_MODELVIEW_MATRIX, modelViewMat );
		invertMatrix( modelViewMat, invModelViewMat );
		glLoadMatrixf( invModelViewMat );

		renderSolidCone( 0.2f, 0.6f, 15, 15 );

		glPopMatrix();
    }
    else if( g_lightType == LIGHT_TYPE_POINT )
    {
		glPushMatrix();
		glLoadIdentity();

		GLfloat pPosition_light2[4];
		glGetLightfv( GL_LIGHT2, GL_POSITION, pPosition_light2 );
		glTranslatef( pPosition_light2[0], pPosition_light2[1], pPosition_light2[2] );

		renderSolidSphere( 0.25f, 15, 15 );

		glPopMatrix();
    }

    // We're finshed with our current light type. Restore the default setup
    // for our next frame, which may use a different light type.
    glPopAttrib();
    
	SwapBuffers( g_hDC );
}
