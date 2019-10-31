//-----------------------------------------------------------------------------
//           Name: ogl_glslang_skinning.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to skin a mesh on the hardware 
//                 using a GLslang shader. To keep things simple, the skeletal 
//                 system used in this sample is very simple and only consists 
//                 of two bones or bone matrices.
//
//   Control Keys: Left Mouse Button  - Spin the matrix for bone0.
//                 Right Mouse Button - Spin the matrix for bone1.
//
//                 F1 - Toggle test geometry between a cylinder and a simple 
//                      grouping of 3 quads.
//                 F2 - Toggle wire-frame mode
//
// NOTE: The sample currently ships with two versions of the shader source. 
//       The first one titled, "ogl_glslang_skinning_nvidia.vert", was my 
//       first attempt to port this shader from Cg to GLslang. Unfortunately, 
//       I unknowingly ended using several Cg only features in my shader and 
//       it was terribly broken on ATI cards! This, of course led to the 
//       second version of the shader file titled, 
//       "ogl_glslang_skinning_ati.vert", which should work on both cards but, 
//       of course, doesn't. So, I'm still working on this one. until then, 
//       just pick the shader source that works the best on your card.
//----------------------------------------------------------------------------- 

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <sys/stat.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "matrix4x4f.h"
#include "vector3f.h"
#include "resource.h"

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
PFNGLVERTEXATTRIB4FVARBPROC      glVertexAttrib4fvARB      = NULL;

// GL_ARB_vertex_shader
PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB = NULL;
PFNGLGETACTIVEATTRIBARBPROC    glGetActiveAttribARB    = NULL;
PFNGLGETATTRIBLOCATIONARBPROC  glGetAttribLocationARB  = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd = NULL;
HDC	   g_hDC  = NULL;
HGLRC  g_hRC  = NULL;
GLuint g_boneDisplayList = -1;

GLhandleARB g_programObj;
GLhandleARB g_vertexShader;
GLuint g_location_boneMatrices_0;
GLuint g_location_boneMatrices_1;
GLuint g_location_eyePosition;
GLuint g_location_lightVector;
GLuint g_location_weights;
GLuint g_location_matrixIndices;
GLuint g_location_numBones;
GLuint g_location_inverseModelView;

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

// Cylinder geometry
const int CYLINDER_RESOLUTION = 12;
const int NUM_CYLINDER_SECTIONS = 6;
const int NUM_VERTICES_PER_SECTION = CYLINDER_RESOLUTION * 2;
const int NUM_VERTICES = NUM_VERTICES_PER_SECTION * NUM_CYLINDER_SECTIONS;

struct Vertex
{
    float r, g, b, a;
    float nx, ny, nz;
    float x, y, z;
	float weights[MAX_BONES];
	float matrixIndices[MAX_BONES];
	float numBones;
};

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
		                     "OpenGL - Skinning on the Hardware With GLslang",
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
    GLfloat position_light0[] = { 0.0f, 0.0f, 1.0f, 0.0f };
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

    //
    // GL_ARB_shading_language_100
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

    //
    // GL_ARB_shader_objects
    //

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
		glVertexAttrib4fvARB      = (PFNGLVERTEXATTRIB4FVARBPROC)wglGetProcAddress("glVertexAttrib4fvARB");

        if( !glCreateProgramObjectARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
            !glCreateShaderObjectARB || !glCreateShaderObjectARB || !glCompileShaderARB || 
            !glGetObjectParameterivARB || !glAttachObjectARB || !glGetInfoLogARB || 
            !glLinkProgramARB || !glGetUniformLocationARB || !glUniform4fARB ||
            !glUniform4fvARB || !glUniform1iARB || !glUniform1fARB || !glUniformMatrix4fvARB ||
            !glVertexAttrib4fvARB )
        {
            MessageBox(NULL,"One or more GL_ARB_shader_objects functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    //
    // GL_ARB_vertex_shader
    //

    if( strstr( ext, "GL_ARB_vertex_shader" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_vertex_shader extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)wglGetProcAddress("glBindAttribLocationARB");
        glGetActiveAttribARB    = (PFNGLGETACTIVEATTRIBARBPROC)wglGetProcAddress("glGetActiveAttribARB");
        glGetAttribLocationARB  = (PFNGLGETATTRIBLOCATIONARBPROC)wglGetProcAddress("glGetAttribLocationARB");

        if( !glBindAttribLocationARB || !glGetActiveAttribARB || !glGetAttribLocationARB )
        {
            MessageBox(NULL,"One or more GL_ARB_vertex_shader functions were not found",
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

    unsigned char *vertexShaderAssembly = readShaderFile( "ogl_glslang_skinning_nvidia.vert" );
    //unsigned char *vertexShaderAssembly = readShaderFile( "ogl_glslang_skinning_ati.vert" );
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

    g_location_inverseModelView = glGetUniformLocationARB( g_programObj, "inverseModelView" );
    g_location_eyePosition      = glGetUniformLocationARB( g_programObj, "eyePosition" );
    g_location_lightVector      = glGetUniformLocationARB( g_programObj, "lightVector" );
    g_location_boneMatrices_0   = glGetUniformLocationARB( g_programObj, "boneMatrices[0]" );
	g_location_boneMatrices_1   = glGetUniformLocationARB( g_programObj, "boneMatrices[1]" );

    g_location_weights        = glGetAttribLocationARB( g_programObj, "weights" );
    g_location_matrixIndices  = glGetAttribLocationARB( g_programObj, "matrixIndices" );
    g_location_numBones       = glGetAttribLocationARB( g_programObj, "numBones" );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
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
//matrix4x4f transposeOfBoneMatrix0;

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

//transposeOfBoneMatrix0 = matrix4x4f::transpose( &g_boneMatrix0 );

    if( g_location_boneMatrices_0 != -1 )
        glUniformMatrix4fvARB( g_location_boneMatrices_0, 1, false, g_boneMatrix0.m );
        //glUniformMatrix4fvARB( g_location_boneMatrices_0, 1, false, transposeOfBoneMatrix0.m );

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
//matrix4x4f transposeOfBoneMatrix1;

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

//transposeOfBoneMatrix1 = matrix4x4f::transpose( &g_boneMatrix1 );

    if( g_location_boneMatrices_1 != -1 )
        glUniformMatrix4fvARB( g_location_boneMatrices_1, 1, false, g_boneMatrix1.m );
        //glUniformMatrix4fvARB( g_location_boneMatrices_1, 1, false, transposeOfBoneMatrix1.m );

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

    // Set the light's directional vector
    if( g_location_lightVector != -1 )
    	glUniform4fARB( g_location_lightVector, fLightVector[0], 
                        fLightVector[1], fLightVector[2], fLightVector[3] );

    // Set the viewer's eye position
    if( g_location_eyePosition != -1 )
    	glUniform4fARB( g_location_eyePosition, fEyePosition[0], 
                        fEyePosition[1], fEyePosition[2], fEyePosition[3] );

    // Set the inverse of the current model-view matrix
    matrix4x4f modelView;
    matrix4x4f inverseModelView;
    glGetFloatv( GL_MODELVIEW_MATRIX, &modelView.m[0] );
	inverseModelView = matrix4x4f::invertMatrix( &modelView );

    if( g_location_inverseModelView != -1 )
        glUniformMatrix4fvARB( g_location_inverseModelView, 1, false, inverseModelView.m );
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
	glTranslatef( 0.0f, -3.0f, g_fDistance ); // Move the view back some...

    glUseProgramObjectARB( g_programObj );

	setShaderConstants();

	if( g_bRenderQuads == true )
	{
		//
		// Render three quads with weighted vertices and skin them using a 
        // GLSlang shader
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
                    glVertexAttrib4fvARB(g_location_weights, fWeights );

					fMatrixIndices[0] = g_quadVertices[nIndex].matrixIndices[0];
					fMatrixIndices[1] = g_quadVertices[nIndex].matrixIndices[1];
                    glVertexAttrib4fvARB(g_location_matrixIndices, fMatrixIndices );

					// Store the number of bones in the x component of a 4 float array
					fNumBones[0] = g_quadVertices[nIndex].numBones; 
                    glVertexAttrib4fvARB(g_location_numBones, fNumBones );

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
		// Render a cylinder with weighted vertices and skin it using a 
        // GLslang shader
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
                    glVertexAttrib4fvARB(g_location_weights, fWeights );

					fMatrixIndices[0] = g_cylinderVertices[nIndex].matrixIndices[0];
					fMatrixIndices[1] = g_cylinderVertices[nIndex].matrixIndices[1];
                    glVertexAttrib4fvARB(g_location_matrixIndices, fMatrixIndices );

					// Store the number of bones in the x component of a 4 float array
					fNumBones[0] = g_cylinderVertices[nIndex].numBones;
                    glVertexAttrib4fvARB(g_location_numBones, fNumBones );

					glColor4f( g_cylinderVertices[nIndex].r, g_cylinderVertices[nIndex].g, g_cylinderVertices[nIndex].b, g_cylinderVertices[nIndex].a );
					glNormal3f( g_cylinderVertices[nIndex].nx, g_cylinderVertices[nIndex].ny, g_cylinderVertices[nIndex].nz );
					glVertex3f( g_cylinderVertices[nIndex].x, g_cylinderVertices[nIndex].y, g_cylinderVertices[nIndex].z );
				}
			}
			glEnd();
		}
	}

    glUseProgramObjectARB( NULL );

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
