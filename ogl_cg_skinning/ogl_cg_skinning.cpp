//-----------------------------------------------------------------------------
//           Name: ogl_cg_skinning.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to skin a mesh on the hardware 
//                 using a Cg shader. To keep things simple, the skeletal 
//                 system used in this sample is very simple and only consists 
//                 of two bones or bone matrices.
//
//                 Special thanks go out to Cyril Zeller, and Matthias Wloka
//                 of nVIDIA for their help in straightening out a few oddities 
//                 that my sample was suffering from. In short, Cg works fine
//                 and I'm occasionally a big dummy! ;)
//
//   Control Keys: Left Mouse Button  - Spin the matrix for bone0.
//                 Right Mouse Button - Spin the matrix for bone1.
//
//                 F1 - Toggle test geometry between a cylinder and a simple 
//                      grouping of 3 quads.
//                 F2 - Toggle wire-frame mode
//----------------------------------------------------------------------------- 

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "matrix4x4f.h"
#include "vector3f.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd = NULL;
HDC	   g_hDC  = NULL;
HGLRC  g_hRC  = NULL;
GLuint g_boneDisplayList = -1;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_modelViewProjection;
CGparameter g_CGparam_modelViewInverse;
CGparameter g_CGparam_boneMatrices;
CGparameter g_CGparam_eyePosition;
CGparameter g_CGparam_lightVector;
CGparameter g_CGparam_weights;
CGparameter g_CGparam_matrixIndices;
CGparameter g_CGparam_numBones;

bool g_bRenderInWireFrame = false;
bool g_bRenderQuads = false;

float g_fDistance = -12.0f;
float g_fSpinX_L  =   0.0f;
float g_fSpinY_L  =   0.0f;
float g_fSpinX_R  =   0.0f;
float g_fSpinY_R  =   0.0f;

const int MAX_BONES = 2;
matrix4x4f g_boneMatrix0;
matrix4x4f g_boneMatrix1;

matrix4x4f g_matrixToRenderBone0;
matrix4x4f g_matrixToRenderBone1;

struct Vertex
{
    float r, g, b, a;
    float nx, ny, nz;
    float x, y, z;
	float weights[MAX_BONES];
	float matrixIndices[MAX_BONES];
	float numBones;
};

// Cylinder geometry
const int CYLINDER_RESOLUTION = 12;
const int NUM_CYLINDER_SECTIONS = 6;
const int NUM_VERTICES_PER_SECTION = CYLINDER_RESOLUTION * 2;
const int NUM_VERTICES = NUM_VERTICES_PER_SECTION * NUM_CYLINDER_SECTIONS;

Vertex g_cylinderVertices[NUM_VERTICES];

// Quad geometry
Vertex g_quadVertices[] =
{
	// r    g    b    a      nx   ny  nz     x    y    z      w0   w1    mi0   mi1    nb
	{ 1.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,0.0f,0.0f,  1.0f,0.0f,  0.0f,0.0f,  1.0f }, // Quad 0
	{ 1.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,0.0f,0.0f,  1.0f,0.0f,  0.0f,0.0f,  1.0f },
	{ 1.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,2.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },
	{ 1.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,2.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },

	{ 0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,2.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f }, // Quad 1
	{ 0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,2.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },
	{ 0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,4.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },
	{ 0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,4.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },

	{ 0.0f,0.0f,1.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,4.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f }, // Quad 2
	{ 0.0f,0.0f,1.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,4.0f,0.0f,  0.5f,0.5f,  0.0f,1.0f,  2.0f },
	{ 0.0f,0.0f,1.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,6.0f,0.0f,  1.0f,0.0f,  1.0f,0.0f,  1.0f },
	{ 0.0f,0.0f,1.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,6.0f,0.0f,  1.0f,0.0f,  1.0f,0.0f,  1.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void initShader(void);
void setShaderConstants(void);
void createMeshCylinderWithWeightedVertices(void);

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
		                     "OpenGL - Skinning on the Hardware Using a Cg Shader",
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
	static POINT ptLastMousePosit_L;
	static POINT ptCurrentMousePosit_L;
	static bool  bMousing_L;

	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;
	
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
					g_bRenderQuads = !g_bRenderQuads;
					break;

				case VK_F2:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                    else
                        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
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
            ptLastMousePosit_L.x = ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y = HIWORD (lParam);
            bMousing_L = true;
        }
        break;

        case WM_LBUTTONUP:
        {
            bMousing_L = false;
        }
        break;

        case WM_RBUTTONDOWN:
        {
            ptLastMousePosit_R.x = ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y = HIWORD (lParam);
            bMousing_R = true;
        }
        break;

        case WM_RBUTTONUP:
        {
            bMousing_R = false;
        }
        break;

        case WM_MOUSEMOVE:
        {
            ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptCurrentMousePosit_L.y = HIWORD (lParam);
            ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptCurrentMousePosit_R.y = HIWORD (lParam);

            if( bMousing_L )
            {
                g_fSpinX_L -= (ptCurrentMousePosit_L.x - ptLastMousePosit_L.x);
                g_fSpinY_L -= (ptCurrentMousePosit_L.y - ptLastMousePosit_L.y);
            }
            
            if( bMousing_R )
            {
                g_fSpinX_R -= (ptCurrentMousePosit_R.x - ptLastMousePosit_R.x);
                g_fSpinY_R -= (ptCurrentMousePosit_R.y - ptLastMousePosit_R.y);
            }

            ptLastMousePosit_L.x = ptCurrentMousePosit_L.x;
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y;
            ptLastMousePosit_R.x = ptCurrentMousePosit_R.x;
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y;
        }
        break;

        case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 1000.0);
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

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_LIGHTING );

    // Set up a material
    GLfloat ambient_mtrl[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse_mtrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_mtrl ); 
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse_mtrl );

    // Set light 0 to be a pure white directional light
    GLfloat diffuse_light0[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat ambient_light0[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat position_light0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambient_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
    glEnable( GL_LIGHT0 );

    // Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
    GLfloat ambient_lightModel[] = { 0.2f, 0.2f, 0.2f, 0.2f };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//
	// Create a piece of geometry to represent a single bone, which is 3.0f 
	// units in length...
	//

	g_boneDisplayList = glGenLists(1);
	glNewList( g_boneDisplayList, GL_COMPILE );
	{
		glBegin( GL_LINE_STRIP );
		{
			glVertex3f( 0.0f, 0.0f, 0.0f);	
			glVertex3f(-0.2f, 0.2f,-0.2f);
			glVertex3f( 0.2f, 0.2f,-0.2f);
			glVertex3f( 0.0f, 3.0f, 0.0f); // Bone length = 3.0f
			glVertex3f(-0.2f, 0.2f,-0.2f);
			glVertex3f(-0.2f, 0.2f, 0.2f);
			glVertex3f( 0.0f, 0.0f, 0.0f);
			glVertex3f( 0.2f, 0.2f,-0.2f);
			glVertex3f( 0.2f, 0.2f, 0.2f);
			glVertex3f( 0.0f, 0.0f, 0.0f);
			glVertex3f(-0.2f, 0.2f, 0.2f);
			glVertex3f( 0.0f, 3.0f, 0.0f); // Bone length = 3.0f
			glVertex3f( 0.2f, 0.2f, 0.2f);
			glVertex3f(-0.2f, 0.2f, 0.2f);
		}
		glEnd();
	}
	glEndList();

	//
	// Create the geometry for a cylinder...
	//

	createMeshCylinderWithWeightedVertices();
}

//-----------------------------------------------------------------------------
// Name: createMeshCylinderWithWeightedVertices()
// Desc: 
//-----------------------------------------------------------------------------
void createMeshCylinderWithWeightedVertices( void )
{
	float fTheta;
	int   nIndex;
	int   nIndexOffset;
	const float fTwoTimesPI = 2 * 3.141592654f;

	//
	// Create the geometry for a cylinder mesh...
	//

	for( int j = 0; j < NUM_CYLINDER_SECTIONS; ++j )
	{
		nIndexOffset = j * NUM_VERTICES_PER_SECTION;

		for( int i = 0; i < CYLINDER_RESOLUTION; ++i )
		{
			fTheta = (fTwoTimesPI * i) / (CYLINDER_RESOLUTION - 1);

			//
			// Top of cylinder section...
			//

			nIndex = (2*i+0) + nIndexOffset;

			g_cylinderVertices[nIndex].x = sinf(fTheta);
			g_cylinderVertices[nIndex].y = (float)j + 1.0f;
			g_cylinderVertices[nIndex].z = cosf(fTheta);

			g_cylinderVertices[nIndex].nx = sinf(fTheta);
			g_cylinderVertices[nIndex].ny = 0.0f;
			g_cylinderVertices[nIndex].nz = cosf(fTheta);

			//
			// Bottom of cylinder section...
			//

			nIndex = (2*i+1) + nIndexOffset;

			g_cylinderVertices[nIndex].x = sinf(fTheta);
			g_cylinderVertices[nIndex].y = (float)j;
			g_cylinderVertices[nIndex].z = cosf(fTheta);

			g_cylinderVertices[nIndex].nx = sinf(fTheta);
			g_cylinderVertices[nIndex].ny = 0.0f;
			g_cylinderVertices[nIndex].nz = cosf(fTheta);
		}
	}

	//
	// Next, assign skinning information to the cylinder's vertex data...
	//
	// The cylinder was built up along the Y axis, so if we vary the 
	// vertex weights based on its Y axis value, we can make it bend like 
	// a plastic pipe. In a real-life application, the weights would most 
	// likely be assigned to each vertex through a modeling tool like Maya or 
	// 3D Studio Max, which is far more intuitive to work with.
	//

	for( int i = 0; i < NUM_VERTICES; ++i )
	{
		// Zero out these to prevent bad data from sneaking in...
		g_cylinderVertices[i].numBones = 0.0f;
		g_cylinderVertices[i].matrixIndices[0] = 0.0f;
		g_cylinderVertices[i].matrixIndices[1] = 0.0f;
		g_cylinderVertices[i].matrixIndices[2] = 0.0f;
		g_cylinderVertices[i].matrixIndices[3] = 0.0f;
		g_cylinderVertices[i].weights[0] = 0.0f;
		g_cylinderVertices[i].weights[1] = 0.0f;
		g_cylinderVertices[i].weights[2] = 0.0f;
		g_cylinderVertices[i].weights[3] = 0.0f;

		// Now, vary the weights for the current vertex based on its 
		// Y axis value...

		if( g_cylinderVertices[i].y < 3.0f )
		{
			//
			// The vertex is in the lower half, which is closet to bone0.
			//
			// Number of bones that effect this vertex = 1
			// Index of bone matrix that effects it    = 0
			// Influence from that bone matrix         = 100%
			//

			g_cylinderVertices[i].numBones = 1.0f;
			g_cylinderVertices[i].matrixIndices[0] = 0.0f;
			g_cylinderVertices[i].weights[0] = 1.0f;

			// Make the vertex yellow as a visual reference...
			g_cylinderVertices[i].r = 1.0f;
			g_cylinderVertices[i].g = 1.0f;
			g_cylinderVertices[i].b = 0.0f;
			g_cylinderVertices[i].a = 1.0f;
		}
		else if( g_cylinderVertices[i].y == 3.0f )
		{
			//
			// The vertex is in the middle, which is equal distance from 
			// bone0 and bone1.
			//
			// Number of bones that effect this vertex         = 2
			// Index of the first bone matrix that effects it  = 0
			// Index of the second bone matrix that effects it = 1
			// Influence from the first bone matrix            = 50%
			// Influence from the second bone matrix           = 50%
			//

			g_cylinderVertices[i].numBones = 2.0f;
			g_cylinderVertices[i].matrixIndices[0] = 0.0f;
			g_cylinderVertices[i].matrixIndices[1] = 1.0f;
			g_cylinderVertices[i].weights[0] = 0.5f;
			g_cylinderVertices[i].weights[1] = 0.5f;

			// Make the vertex green as a visual reference...
			g_cylinderVertices[i].r = 0.0f;
			g_cylinderVertices[i].g = 1.0f;
			g_cylinderVertices[i].b = 0.0f;
			g_cylinderVertices[i].a = 1.0f;
		}
		else if( g_cylinderVertices[i].y > 3.0f )
		{
			//
			// The vertex is in the upper half, which is closet to bone1.
			//
			// Number of bones that effect this vertex = 1
			// Index of bone matrix that effects it    = 1
			// Influence from that bone matrix         = 100%
			//

			g_cylinderVertices[i].numBones = 2.0f;
			g_cylinderVertices[i].matrixIndices[0] = 0.0f;
			g_cylinderVertices[i].matrixIndices[1] = 1.0f;
			g_cylinderVertices[i].weights[0] = 0.0f;
			g_cylinderVertices[i].weights[1] = 1.0f;

			// Make the vertex blue as a visual reference...
			g_cylinderVertices[i].r = 0.0f;
			g_cylinderVertices[i].g = 0.0f;
			g_cylinderVertices[i].b = 1.0f;
			g_cylinderVertices[i].a = 1.0f;
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
	// CG_PROFILE_VP30   - GL_NV_vertex_program2 (supports for loops)
	// CG_PROFILE_ARBVP1 - GL_ARB_vertex_program (doesn't support for loops)
	// CG_PROFILE_VP20   - GL_NV_vertex_program
    //

    if( cgGLIsProfileSupported(CG_PROFILE_VP30) )
        g_CGprofile = CG_PROFILE_VP30;
    else if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        g_CGprofile = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP20) )
        g_CGprofile = CG_PROFILE_VP20;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
			        "support any of the vertex shading extensions!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	// Create the vertex shader...
	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
										   CG_SOURCE,
										   "ogl_cg_skinning.cg",
										   g_CGprofile,
										   NULL, 
										   NULL );

	// Load the program using Cg's expanded interface...
	cgGLLoadProgram( g_CGprogram );

	// Bind some parameters by name so we can set them later...
	g_CGparam_modelViewProjection = cgGetNamedParameter(g_CGprogram, "modelViewProjection");
	g_CGparam_modelViewInverse = cgGetNamedParameter(g_CGprogram, "modelViewInverse");
	g_CGparam_boneMatrices = cgGetNamedParameter(g_CGprogram, "boneMatrices");
	g_CGparam_eyePosition = cgGetNamedParameter(g_CGprogram, "eyePosition");
	g_CGparam_lightVector = cgGetNamedParameter(g_CGprogram, "lightVector");

    // These vary on a per-vertex basis...
    g_CGparam_weights = cgGetNamedParameter(g_CGprogram, "IN.weights");
    g_CGparam_matrixIndices = cgGetNamedParameter(g_CGprogram, "IN.matrixIndices");
    g_CGparam_numBones = cgGetNamedParameter(g_CGprogram, "IN.numBones");
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
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
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, -3.0f, g_fDistance ); // Move the view back some...

	// Track the combined model-view-projection matrix
	cgGLSetStateMatrixParameter( g_CGparam_modelViewProjection,
							     CG_GL_MODELVIEW_PROJECTION_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	// This matrix will be used to transform the normals from model-space to view-space
	cgGLSetStateMatrixParameter( g_CGparam_modelViewInverse,
							     CG_GL_MODELVIEW_MATRIX,
                                 CG_GL_MATRIX_INVERSE_TRANSPOSE );

	//
	// Set the matrix for bone0...
	//
	// The matrix for bone0 is fairly simple since it's the first bone in the 
	// hierarchy. To liven things up a bit, we'll have bone0 slowly translate 
	// left and right. This will help to demonstrate how bone1, which is attached 
	// to the end of bone0, moves in relation to it.
	//

	g_boneMatrix0.identity();
	g_matrixToRenderBone0.identity();

	matrix4x4f translationMatrixX;
	matrix4x4f rotationMatrixY;
	matrix4x4f rotationMatrixZ;
	matrix4x4f boneRotationMatrix;
	matrix4x4f transposeOfBoneMatrix0;

	// As a demonstration, translate bone0 left and right...
	float fTime = (float)GetTickCount() / 2000.0f;
	translationMatrixX.translate_x( sinf( fTime ) );

	// Build up a rotation matrix for bone0...
	rotationMatrixY.rotate_y( g_fSpinY_L);
	rotationMatrixZ.rotate_z(-g_fSpinX_L);
	boneRotationMatrix = rotationMatrixY * rotationMatrixZ;

	g_boneMatrix0 = boneRotationMatrix * translationMatrixX;

	// Cache what we have so far so we can render the graphical representation 
	// of bone0 correctly. This is just for the demo and is not required for 
    // mesh skinning.
	g_matrixToRenderBone0 = g_boneMatrix0;

	transposeOfBoneMatrix0 = matrix4x4f::transpose( &g_boneMatrix0 );

	CGparameter CGparam_currentMatrix = cgGetArrayParameter( g_CGparam_boneMatrices, 0 );
	cgGLSetMatrixParameterfr( CGparam_currentMatrix, transposeOfBoneMatrix0.m );

    //
    // Set the matrix for bone1...
    //
    // The setup for bone1 is more complicated than bone0. This is for two 
    // reasons. 
    //
    // First, bone0 and bone1 are supposed to be connected at a joint, 
    // so we need to make sure that bone1 follows along with any translation 
    // and/or rotation that bone0 performs. If we don't, the mesh will be 
    // stretched out as the bones move apart.
    //
    // Secondly, bone1 performs its rotation at the tip-end of bone0, so we 
    // need to take into account the length of bone0 (3.0 units along Y) when 
    // rotating bone1, or the vertices weighted to respond to bone1 will get 
    // rotated strangely. When it comes to mesh skinning, this is probably the 
    // hardest part to understand.
    //

	g_boneMatrix1.identity();
	g_matrixToRenderBone1.identity();

	matrix4x4f offsetMatrix_toBoneEnd;
	matrix4x4f offsetMatrix_backFromBoneEnd;
	matrix4x4f transposeOfBoneMatrix1;

    // Build up two offset matrices for bone1...
	offsetMatrix_toBoneEnd.translate_y( 3.0f );
	offsetMatrix_backFromBoneEnd.translate_y( -3.0f );

	// Build up a rotation matrix for bone1...
	rotationMatrixY.rotate_y( g_fSpinY_R);
	rotationMatrixZ.rotate_z(-g_fSpinX_R);
	boneRotationMatrix = rotationMatrixY * rotationMatrixZ;

	// g_boneMatrix0          = Move in relation to bone0
	// offsetMatrix_toBoneEnd = Offset to the end of bone0
	// boneRotationMatrix     = Apply rotation of bone1
	g_boneMatrix1 = g_boneMatrix0 * offsetMatrix_toBoneEnd * boneRotationMatrix;

	// Cache what we have so far so we can render the graphical representation 
	// of bone1 correctly. This is just for the demo and is not required for mesh skinning.
	g_matrixToRenderBone1 = g_boneMatrix1;

	// Once rotated, undo the translation offset so it doesn't translate the 
	// actual vertices...
	g_boneMatrix1 = g_boneMatrix1 * offsetMatrix_backFromBoneEnd;

	transposeOfBoneMatrix1 = matrix4x4f::transpose( &g_boneMatrix1 );

	CGparam_currentMatrix = cgGetArrayParameter( g_CGparam_boneMatrices, 1 );
	cgGLSetMatrixParameterfr( CGparam_currentMatrix, transposeOfBoneMatrix1.m );

	//
	// Set up some lighting...
	//

	float fEyePosition[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float fLightVector[] = { 0.0f, 0.0f, -1.0f, 0.0f };

	// Normalize light vector
    float fLength = sqrtf( fLightVector[0]*fLightVector[0] +
                           fLightVector[1]*fLightVector[1] +
                           fLightVector[2]*fLightVector[2] );
	fLightVector[0] /= fLength;
	fLightVector[1] /= fLength;
	fLightVector[2] /= fLength;

    cgGLSetParameter4fv( g_CGparam_eyePosition, fEyePosition );
	cgGLSetParameter4fv( g_CGparam_lightVector, fLightVector );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	setShaderConstants();

	cgGLBindProgram( g_CGprogram );
	cgGLEnableProfile( g_CGprofile );

	if( g_bRenderQuads == true )
	{
		//
		// Render three quads with weighted vertices and skin them using a Cg shader
		//

		int nIndex;
		int nIndexOffset;

		float fWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };
		float fNumBones[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };
		float fMatrixIndices[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };

		// For each quad...
		for( int j = 0; j < 3; ++j )
		{
			nIndexOffset = j * 4;

			glBegin( GL_QUADS );
			{
				// Render the quad's four vertices...
				for( int i = 0; i < 4; ++i )
				{
					nIndex = i + nIndexOffset;

					fWeights[0] = g_quadVertices[nIndex].weights[0];
					fWeights[1] = g_quadVertices[nIndex].weights[1];
					cgGLSetParameter4fv( g_CGparam_weights, fWeights );

					fMatrixIndices[0] = g_quadVertices[nIndex].matrixIndices[0];
					fMatrixIndices[1] = g_quadVertices[nIndex].matrixIndices[1];
					cgGLSetParameter4fv( g_CGparam_matrixIndices, fMatrixIndices );

					// Store the number of bones in the x component of a 4 float array
					fNumBones[0] = g_quadVertices[nIndex].numBones; 
					cgGLSetParameter4fv( g_CGparam_numBones, fNumBones );

					glColor4f( g_quadVertices[nIndex].r, g_quadVertices[nIndex].g, g_quadVertices[nIndex].b, g_quadVertices[nIndex].a );
					glNormal3f( g_quadVertices[nIndex].nx, g_quadVertices[nIndex].ny, g_quadVertices[nIndex].nz );
					glVertex3f( g_quadVertices[nIndex].x, g_quadVertices[nIndex].y, g_quadVertices[nIndex].z );
				}
			}
			glEnd();
		}
	}
	else
	{
		//
		// Render a cylinder with weighted vertices and skin it using a Cg shader
		//

		int nIndex;
		int nIndexOffset;

		float fWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };
		float fNumBones[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };
		float fMatrixIndices[4] = { 0.0f, 0.0f, 0.0f, 0.0f, };

		// For each cylinder section...
		for( int j = 0; j < NUM_CYLINDER_SECTIONS; ++j )
		{
			nIndexOffset = j * NUM_VERTICES_PER_SECTION;

			glBegin( GL_TRIANGLE_STRIP );
			{
				// Render the section as a tri-strip...
				for( int i = 0; i < NUM_VERTICES_PER_SECTION; ++i )
				{
					nIndex = i + nIndexOffset;

					fWeights[0] = g_cylinderVertices[nIndex].weights[0];
					fWeights[1] = g_cylinderVertices[nIndex].weights[1];
					cgGLSetParameter4fv( g_CGparam_weights, fWeights );

					fMatrixIndices[0] = g_cylinderVertices[nIndex].matrixIndices[0];
					fMatrixIndices[1] = g_cylinderVertices[nIndex].matrixIndices[1];
					cgGLSetParameter4fv( g_CGparam_matrixIndices, fMatrixIndices );

					// Store the number of bones in the x component of a 4 float array
					fNumBones[0] = g_cylinderVertices[nIndex].numBones;
					cgGLSetParameter4fv( g_CGparam_numBones, fNumBones );

					glColor4f( g_cylinderVertices[nIndex].r, g_cylinderVertices[nIndex].g, g_cylinderVertices[nIndex].b, g_cylinderVertices[nIndex].a );
					glNormal3f( g_cylinderVertices[nIndex].nx, g_cylinderVertices[nIndex].ny, g_cylinderVertices[nIndex].nz );
					glVertex3f( g_cylinderVertices[nIndex].x, g_cylinderVertices[nIndex].y, g_cylinderVertices[nIndex].z );
				}
			}
			glEnd();
		}
	}

	cgGLDisableProfile( g_CGprofile );

	//
	// Render the graphical representations of our two bones for reference...
	//

	glDisable( GL_LIGHTING );

	glPushMatrix();
	{
		glMultMatrixf( g_matrixToRenderBone0.m );
		glColor3f( 1.0f, 1.0f, 0.0 ); // Bone0 will be green
		glCallList( g_boneDisplayList );
	}
	glPopMatrix();

	glPushMatrix();
	{
		glMultMatrixf( g_matrixToRenderBone1.m );
		glColor3f( 0.0f, 0.0f, 1.0 ); // Bone1 will be blue
		glCallList( g_boneDisplayList );
	}
	glPopMatrix();

	glEnable( GL_LIGHTING );

	SwapBuffers( g_hDC );
}
