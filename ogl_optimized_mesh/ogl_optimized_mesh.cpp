//-----------------------------------------------------------------------------
//           Name: ogl_optimized_mesh.cpp
//         Author: Kevin Harris
//  Last Modified: 04/25/06
//    Description: This sample demonstrates how to optimize a regular triangle
//                 mesh by using a combination of indexed geometry, tri-strips,
//                 and degenerate triangles.
//
//   Control Keys: Up   - View moves forward
//                 Down - View moves backward
//
//                 F1 - Toggle between a regular mesh composed of simple 
//                      triangles or a highly optimized indexed mesh composed   
//                      of multiple tri-strips combined together with  
//                      degenerate triangles.
//
//                 F2 - Toggle wire-frame mode. Note, if you're currently 
//                      viewing the optimized mesh, this will let you see the 
//                      degenerate triangles used to combine together 
//                      tri-strips.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "bitmap_fonts.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC           = NULL;
HGLRC     g_hRC           = NULL;
HWND      g_hWnd          = NULL;
HINSTANCE g_hInstance     = NULL;
int       g_nWindowWidth  = 640;
int       g_nWindowHeight = 480;
GLuint    g_textureID     = -1;

bool  g_bFirstRendering = true;
timeb g_lastTime;
float g_fTimeSinceLastReport = 0.0;
int   g_nFrameCount = 0;

bool g_bUseIndexedMesh = true;
bool g_bWireFrameMode  = false;

float g_fDistance = -4.0f;
float g_fSpinX    =  0.0f;
float g_fSpinY    =  0.0f;

struct Vertex
{
	// GL_T2F_C4F_N3F_V3F
	float tu, tv;
	float r, g, b, a;
	float nx, ny, nz;
	float x, y, z;
};

//
// Mesh properties...
//

const int   g_nNumVertsAlongX   = 25;
const int   g_nNumVertsAlongZ   = 25;
const float g_fMeshLengthAlongX = 2.0f;
const float g_fMeshLengthAlongZ = 2.0f;


//
// -- Regular mesh --
//
// Composed of simple triangles.
//

// Number of vertices required for the mesh
const int g_nRegularVertCount = (g_nNumVertsAlongX-1) * (g_nNumVertsAlongZ-1) * 6;
Vertex g_regularMeshVertices[g_nRegularVertCount];


//
// -- Indexed mesh --
//
// Composed of multiple tri-strips combined together with degenerate triangles.
//

// Number of vertices required for the indexed mesh
const int g_nIndexedVertCount = g_nNumVertsAlongX * g_nNumVertsAlongZ;
Vertex g_indexedMeshVertices[g_nIndexedVertCount];

// Number of indices required for the indexed mesh
const int g_nIndicesCount = (g_nNumVertsAlongX*2) * (g_nNumVertsAlongZ-1) + ((g_nNumVertsAlongZ-2)*2);
GLuint g_meshIndices[g_nIndicesCount];

//-----------------------------------------------------------------------------
// To understand how tri-strips can be joined together with degenerate 
// triangles, lets take a look at a simple indexed mesh which is defined by two 
// tri-strips with one on top of the other:
//
//                  6---7---8
//                  |  /|  /|
// tri-strip #2 ->  | / | / |
//                  |/  |/  |
//                  3---4---5
//                  |  /|  /|
//                  | / | / |
// tri-strip #1 ->  |/  |/  |
//                  0---1---2
//
// Here's what the separate indices array would look like for these two 
// tri-strips:
//
//    tri-strip #1          tri-strip #2
//                     
//  3, 0, 4, 1, 5, 2,     6, 3, 7, 4, 8, 5
//
// While the mesh is already some what optimized through the usage of indexed 
// geometry and tri-strips, we still have to make two separate glDrawElements 
// calls to render them. This is because we need two separate indices array to 
// render them correctly. If we just combine the two indices arrays, OpenGL 
// wouldn't be able to tell where the first one ends and the second one begins
// and a strange piece of geometry would get rendered by OpenGL between the 
// first and second strips.
//
// To fix this, we'll insert a degenerate triangle which will combine the two 
// tri-strips together in such a way as to allow them to be safely called as 
// one long tri-strip. A degenerate triangle is called "degenerate" because it 
// has no surface area and generates no pixels when rendered. In other words,
// it's an invisible bridge that joins our two strips together.
//
// Here's how to insert a degenerate triangle between the two tri-strips 
// (i.e. between vertices 2 and 6):
//
//                   3---4---5   6---7---8
//                   |  /|  /|  /|  /|  /|
//  tri-strip #1 ->  | / | / | / | / | / |  <- tri-strip #2
//                   |/  |/  |/  |/  |/  |
//                   0---1---2   3---4---5
//
// NOTE: To show the degenerate triangle, I'm placing the two tri-strips side 
//       by instead of one on top of the other like they actually are in the
//       mesh. See the diagram at the top of this comment block for the actual 
//       mesh layout.
//
// When we reach the end of the first tri-strip, we start a degenerate triangle 
// by repeating the first strip's last index again, which is 2 in this case.
// Next, we repeat the next strip's first index, which is 6. This inserts a 
// degenerate triangle that joins the first tri-strip with the second one.
//
// With that done, our indices array looks like this:
//
//    tri-strip #1      +    degenerate triangle    +      tri-strip #2
//
//  3, 0, 4, 1, 5, 2,   +           2, 6,           +    6, 3, 7, 4, 8, 5
//
// Or, as one long tri-strip...
//
// 3, 0, 4, 1, 5, 2, 2, 6, 6, 3, 7, 4, 8, 5
//
// And now we can render both tri-strips with one OpenGL call with out any 
// graphical anomalies!
//-----------------------------------------------------------------------------

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
void createRegularMeshUsingTriangles(void);
void createIndexedMeshUsingTriStrips(void);

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
						    "OpenGL - Simple Triangle Mesh  VS. Indexed Mesh Using Tri-Strips and Degenerate Triangles",
                            WS_OVERLAPPEDWINDOW, 0, 0, g_nWindowWidth, g_nWindowHeight, 
                            NULL, NULL, g_hInstance, NULL );

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

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

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
					g_bUseIndexedMesh = !g_bUseIndexedMesh;
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
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\mandrill.bmp" );

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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );

    loadTexture();
    createRegularMeshUsingTriangles();
	createIndexedMeshUsingTriStrips();
}

//-----------------------------------------------------------------------------
// Name: createRegularMeshUsingTriangles()
// Desc: 
//-----------------------------------------------------------------------------
void createRegularMeshUsingTriangles( void )
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
	for( i = 0; i < g_nRegularVertCount; ++i )
	{
		// Mesh tesselation occurs in the X,Z plane, so Y is always zero
		g_regularMeshVertices[i].y = 0.0f;

		g_regularMeshVertices[i].nx = 0.0f;
		g_regularMeshVertices[i].ny = 1.0f;
		g_regularMeshVertices[i].nz = 0.0f;

		g_regularMeshVertices[i].r = 1.0f;
		g_regularMeshVertices[i].g = 1.0f;
		g_regularMeshVertices[i].b = 1.0f;
	}

	//
	// Create all the vertex points required by the mesh...
	//
	// Note: Mesh tessellation occurs in the X,Z plane.
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
			g_regularMeshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_regularMeshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_regularMeshVertices[i].tu = x * dTU;
			g_regularMeshVertices[i].tv = z * dTV;
			++i;

			// 1
			g_regularMeshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_regularMeshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_regularMeshVertices[i].tu = (x+1.0f) * dTU;
			g_regularMeshVertices[i].tv = z * dTV;
			++i;

			// 2
			g_regularMeshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_regularMeshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_regularMeshVertices[i].tu = (x+1.0f) * dTU;
			g_regularMeshVertices[i].tv = (z+1.0f) * dTV;
			++i;

			// Second triangle of the current quad
			// 2 ___ 1
			//  |  /|
			//  |/__|
			// 0

			// 0
			g_regularMeshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_regularMeshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_regularMeshVertices[i].tu = x * dTU;
			g_regularMeshVertices[i].tv = z * dTV;
			++i;

			// 1
			g_regularMeshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
			g_regularMeshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_regularMeshVertices[i].tu = (x+1.0f) * dTU;
			g_regularMeshVertices[i].tv = (z+1.0f) * dTV;
			++i;

			// 2
			g_regularMeshVertices[i].x = g_fMeshLengthAlongX * x * dX;
			g_regularMeshVertices[i].z = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
			g_regularMeshVertices[i].tu = x * dTU;
			g_regularMeshVertices[i].tv = (z+1.0f) * dTV;
			++i;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: createIndexedMeshUsingTriStrips()
// Desc: 
//-----------------------------------------------------------------------------
void createIndexedMeshUsingTriStrips( void )
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
	for( i = 0; i < g_nIndexedVertCount; ++i )
	{
		// Mesh tessellation occurs in the X,Z plane, so Y is always zero
		g_indexedMeshVertices[i].y = 0.0f;

		g_indexedMeshVertices[i].nx = 0.0f;
		g_indexedMeshVertices[i].ny = 1.0f;
		g_indexedMeshVertices[i].nz = 0.0f;

		g_indexedMeshVertices[i].r = 1.0f;
		g_indexedMeshVertices[i].g = 1.0f;
		g_indexedMeshVertices[i].b = 1.0f;
	}

	//
    // Create all the vertex points required by the mesh...
    //
	// Note: Mesh tessellation occurs in the X,Z plane.
	//

	// For every vertex along the Z axis...
	for( z = 0, i = 0; z < g_nNumVertsAlongZ; ++z )
	{
		// ... create a row of vertex points along the X axis
		for( x = 0; x < g_nNumVertsAlongX; ++x )
		{
			g_indexedMeshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
			g_indexedMeshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
			g_indexedMeshVertices[i].tu = x * dTU;
			g_indexedMeshVertices[i].tv = z * dTV;
			++i;
		}
	}

	//
	// Now that we have the vertices, let's fill in our indices array with 
    // values which will reference the vertices we created earlier. 
    //
    // The nested for() loops below will define our mesh as a series of 
    // tri-strips, which are connected together via degenerate triangles. This
    // is what allows us to render them all with one call instead of making a 
    // series of separate calls to render each tri-strip.
	//

    // Zero out the indices array...
	for( i = 0; i < g_nIndicesCount; ++i )
		g_meshIndices[i] = 0;

	// For each row of our mesh...
	for( z = 0, i = 0; z < g_nNumVertsAlongZ-1; ++z )
	{
		// ...define a tri-strip that defines the row's geometry.
		for( x = 0; x < g_nNumVertsAlongX; ++x )
		{
			// I---.---. First, set the upper index (I) here
			// |  /|  /|
			// | / | / |
			// |/  |/  |
			// .---.---.

			g_meshIndices[i] = x + ( g_nNumVertsAlongX * (z+1) );
			++i;

			// .---.---.
			// |  /|  /|
			// | / | / |
			// |/  |/  |
			// I---.---. Next, set the lower index (I)

			g_meshIndices[i] = x + ( g_nNumVertsAlongX * z );
			++i;

			// Keep zig-zagging back and forth until the tri-strip is finished.

            // If we've reached the end of the current tri-strip, insert a 
            // degenerate triangle here which ties the end of this tri-strip  
            // to the start of the next one.
			if( x == g_nNumVertsAlongX-1 )
			{
                // Add a redundant vertex from the END of the CURRENT tri-strip.
				g_meshIndices[i] = x + ( g_nNumVertsAlongX * z );
				i++;

                // Add a redundant vertex from the START of NEXT tri-strip.
				g_meshIndices[i] = ( g_nNumVertsAlongX * (z+2) );
				i++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );

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
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glTranslatef( 0.0f, 0.0f, g_fDistance );
    glRotatef( -(g_fSpinY - 90.0f), 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );
    // Since the mesh is created off-center in the XZ plane we'll need to move 
    // it to the center for it to spin correctly in the sample.
    glTranslatef( -(g_fMeshLengthAlongX/2.0f), 0.0f, (g_fMeshLengthAlongZ/2.0f) );

    // Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if( g_bWireFrameMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	//
	// Render tessellated mesh...
	//

	if( g_bUseIndexedMesh == true )
	{
		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_indexedMeshVertices );
		glDrawElements( GL_TRIANGLE_STRIP, g_nIndicesCount, GL_UNSIGNED_INT, g_meshIndices );
	}
	else
	{
        glBindTexture( GL_TEXTURE_2D, g_textureID );

        glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_regularMeshVertices );
        glDrawArrays( GL_TRIANGLES, 0, g_nRegularVertCount );
	}

	//
	// Report frames-per-second and other settings...
	//

    timeb currentTime;
    float fElapsed = 0.0f;

    if( g_bFirstRendering == true )
    {
        ftime( &g_lastTime );
        currentTime = g_lastTime;
        g_bFirstRendering = false;
    }
    else
    {
        ftime( &currentTime );

        // This is accurate to one second
        fElapsed  = (float)(currentTime.time - g_lastTime.time);
        // This gets it down to one ms
        fElapsed += (float)((currentTime.millitm - g_lastTime.millitm) / 1000.0f);
    }

    static char fpsString[50];

    ++g_nFrameCount;

    // Has one second passed?
    if( fElapsed - g_fTimeSinceLastReport > 1.0f )
    {
        sprintf( fpsString, "Frames Per Second = %d", g_nFrameCount );

        g_fTimeSinceLastReport = fElapsed;
        g_nFrameCount = 0;
    }

    static char meshTypeString[255];

	if( g_bUseIndexedMesh == true )
		sprintf( meshTypeString, "Mesh type = Indexed mesh using tri-strips joined by degenerate triangles (Change: F1)" );
	else
		sprintf( meshTypeString, "Mesh type = Simple triangle mesh (Change: F1)" );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
		glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, meshTypeString );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, fpsString );
	}
	endRenderText();


	SwapBuffers( g_hDC );
}
