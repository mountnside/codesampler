//------------------------------------------------------------------------------
//           Name: oglu_3ds_model_loading.cpp
//         Author: Anudhyan Boral
//  Last Modified: 02/27/06
//    Description: This sample loads and displays a 3ds model
//
//   Control Keys: F1 - Toggles wire frame mode
//
//  Note: The 3ds object "box.3ds" is loaded in init();
//		  This sample has been coded on top of "ogl_initialization"
//		  This sample uses the stone texture from "ogl_dot3_bump_mapping" 
//		  In order to load the 3ds object properly, all geometry should be
//		  attached together. Many other chunks have to be inserted into
//		  Load3dsObject() in order to load data such as lights,vertex colors
//		  multiple objects,material info etc.This code has been tested on
//		  3ds objects from 3dsMax 4,6 and 7...
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>		
#include "resource.h"
#include <io.h>


//-----------------------------------------------------------------------------
// CLASSES
//-----------------------------------------------------------------------------

//Basic Classes:
//Required for representing the loaded model
class vertex{
public:
	float x,y,z;
};

//MapCoord - for storing texture mapping coords
class mapcoord{
public:
	float u,v;
};

//The three ints for the polygon
//represent the no.s(or rank) of it's 3 vertices
class polygon{
public:
	int a,b,c;
};

//Our object consists of a name,the vertex and polygons quantity,
//3000 vertices,3000 polygons, and 3000 mapping coords
//The value 3000 should be increased in order to load high-res models.
class object{
public:
	char name[20];
	int numVerts,numPolys;
	vertex v[3000];
	polygon p[3000];
	mapcoord m[3000];
};

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

bool g_bRenderInWireFrame = false;
unsigned int g_textureID = 1;

object g_Obj3ds;  //our object

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void Load3dsObject(object *, char *);
void DrawObject(object *);

//-----------------------------------------------------------------------------
// Name: Load3dsObject (object *, char *)
// Desc: Loads a 3ds object given a filename
//-----------------------------------------------------------------------------
void Load3dsObject (object *obj, char *filename)
{
	FILE *file;			//Our file pointer

	char temp;			//Temporary char for reading name of object
	short chunkID;		//Stores ID of current chunk.
	int chunkLength;

	short useless;
	
	//Open our file for reading(r) and in binary mode(b)- "rb"
	file=fopen (filename, "rb");

	int i; 

	//While current position is lesser than the total length
	while (ftell(file) < filelength (fileno (file)))  
	{
		fread (&chunkID, 2, 1, file); 
		fread (&chunkLength, 4, 1,file); 

		switch (chunkID)
        {
			case 0x4d4d:		//Skip these chunks
				break;    
			case 0x3d3d:
				break;
			
			case 0x4000:		//Chunk containing name
				for(i=0;i<20;i++)
				{
					fread (&temp, 1, 1, file);
                    obj->name[i]=temp;
					if(temp == '\0')break;
                }
				break;

			case 0x3f80:		//Skip again
				break;
			case 0x4100:
				break;
	
			case 0x4110:		//Chunk with num of vertices
								//followed by their coordinates
				fread (&obj->numVerts, sizeof (unsigned short), 1, file);
				for (i=0; i<obj->numVerts; i++)
                {
					fread (&obj->v[i].x, sizeof(float), 1, file);
 				
                    fread (&obj->v[i].y, sizeof(float), 1, file);
 					
					fread (&obj->v[i].z, sizeof(float), 1, file);
 				
				}
				break;

			case 0x4120:		 //Chunk with numPolys and
								 //the indices
				fread (&obj->numPolys, sizeof (unsigned short), 1, file);
                for (i=0; i<obj->numPolys; i++)
                {
					fread (&obj->p[i].a, sizeof (unsigned short), 1, file);
				
					fread (&obj->p[i].b, sizeof (unsigned short), 1, file);
				
					fread (&obj->p[i].c, sizeof (unsigned short), 1, file);
					
					fread (&useless, sizeof (unsigned short), 1, file);
					
				}
				break;

		
			case 0x4140:		//Chunk with texture coords
				fread (&useless, sizeof (unsigned short), 1, file);
				for (i=0; i<obj->numVerts; i++)
				{
					fread (&obj->m[i].u, sizeof (float), 1, file);
				
                    fread (&obj->m[i].v, sizeof (float), 1, file);
				}
                break;

			default:
				 fseek(file,chunkLength-6, SEEK_CUR);
        } 
	}
	fclose (file);		
}

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
                             "OpenGL - Loading and Viewing 3ds Objects",
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
					g_bRenderInWireFrame=!g_bRenderInWireFrame;
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
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//Loading 3ds object here//
	Load3dsObject(&g_Obj3ds,"box.3ds");

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\stone_wall.bmp" );

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
// Name: DrawObject(object *)
// Desc: Draws the given 3ds object
//-----------------------------------------------------------------------------
void DrawObject(object *obj)
{
	glBegin(GL_TRIANGLES);				//3ds models consist of triangles
    for (int i=0;i<obj->numPolys;i++)	//For each triangle
    {
 		glTexCoord2f( obj->m[ obj->p[i].a ].u,	//1st vertex:Mapping coordinates
                      obj->m[ obj->p[i].a ].v);
        glVertex3f( obj->v[ obj->p[i].a ].x,	//1st vertex: x
                    obj->v[ obj->p[i].a ].y,	//1st vertex: y
                    obj->v[ obj->p[i].a ].z);	//1st vertex: z

		glTexCoord2f( obj->m[ obj->p[i].b ].u,	//2nd vertex:Mapping coordinates
                      obj->m[ obj->p[i].b ].v);
        glVertex3f( obj->v[ obj->p[i].b ].x,	//2nd vertex: x
                    obj->v[ obj->p[i].b ].y,	//2nd vertex: y
                    obj->v[ obj->p[i].b ].z);	//2nd vertex: z

		glTexCoord2f( obj->m[ obj->p[i].c ].u,	//3rd vertex:Mapping coordinates
                      obj->m[ obj->p[i].c ].v);
        glVertex3f( obj->v[ obj->p[i].c ].x,	//3rd vertex: x
                    obj->v[ obj->p[i].c ].y,	//3rd vertex: y
                    obj->v[ obj->p[i].c ].z);	//3rd vertex: z
	}
    glEnd();

}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render(void)	
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -20.0f );

    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );

    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	if(g_bRenderInWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		
	//Draw our object
	DrawObject(&g_Obj3ds);

	SwapBuffers( g_hDC );
}
