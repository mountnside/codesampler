//-----------------------------------------------------------------------------
//           Name: oglgh_rigid_body_dynamics.cpp
//        Authors: Anton Ephanov and Kevin Harris
//  Last Modified: 12/08/05
//    Description: This sample has been developed as a supplementary material
//                 for the "Math and Physics for game developers" class 
//                 at Guildhall SMU (guildhall.smu.edu).
//                 The sample demonstrates integration of 3D rigid body dynamics
//                 equations.
//                 See RigidBodySimulation.h for more details.
//
//   Control Keys: t,T - Toggle between the two rigid body equations. 
//                       The "red" body represents the WRF equation, while
//                       the "blue" body represents the BRF equation.
//                       See RigidBodyEquationWRF.h for more details.
//                 1-6 - Toggle among various spring attachment points
//                 d,D - decrease/increase the viscous damping coefficient that
//                       simulates air friction
//                 k,K - decrease/increase the spring stiffness
//                 SPACE - reset the simulation
//                 See RigidBodyRenderer::onKeyInput for more details.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "tga.h"
#include "resource.h"
#include "RigidBodySimulation.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd      = NULL;
HDC	   g_hDC       = NULL;
HGLRC  g_hRC       = NULL;
GLuint g_textureID = -1;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Renders the cube and the spring
struct RigidBodyRenderer {

    struct Vertex
    {
        Vertex(float vu, float vv, float vx, float vy, float vz) :
        tu(vu),tv(vv),x(vx),y(vy),z(vz){}

        float tu, tv;
        float x, y, z;
    };

    RigidBodyRenderer() : m_currentEquation(RigidBodySimulation::RIGID_BODY_EQUATION_WRF)
    {
        m_cubeVertices = static_cast<Vertex*>(malloc(24 * sizeof(Vertex)));

        float dimf = (float) m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation)/2.0f;
        Vertex* it = m_cubeVertices;

        *it++ = Vertex( 0.0f,0.0f, -dimf,-dimf, dimf );
        *it++ = Vertex( 1.0f,0.0f,  dimf,-dimf, dimf );
        *it++ = Vertex( 1.0f,1.0f,  dimf, dimf, dimf );
        *it++ = Vertex( 0.0f,1.0f, -dimf, dimf, dimf );

        *it++ = Vertex( 1.0f,0.0f, -dimf,-dimf,-dimf );
        *it++ = Vertex( 1.0f,1.0f, -dimf, dimf,-dimf );
        *it++ = Vertex( 0.0f,1.0f,  dimf, dimf,-dimf );
        *it++ = Vertex( 0.0f,0.0f,  dimf,-dimf,-dimf );

        *it++ = Vertex( 0.0f,1.0f, -dimf, dimf,-dimf );
        *it++ = Vertex( 0.0f,0.0f, -dimf, dimf, dimf );
        *it++ = Vertex( 1.0f,0.0f,  dimf, dimf, dimf );
        *it++ = Vertex( 1.0f,1.0f,  dimf, dimf,-dimf );

        *it++ = Vertex( 1.0f,1.0f, -dimf,-dimf,-dimf );
        *it++ = Vertex( 0.0f,1.0f,  dimf,-dimf,-dimf );
        *it++ = Vertex( 0.0f,0.0f,  dimf,-dimf, dimf );
        *it++ = Vertex( 1.0f,0.0f, -dimf,-dimf, dimf );

        *it++ = Vertex( 1.0f,0.0f,  dimf,-dimf,-dimf );
        *it++ = Vertex( 1.0f,1.0f,  dimf, dimf,-dimf );
        *it++ = Vertex( 0.0f,1.0f,  dimf, dimf, dimf );
        *it++ = Vertex( 0.0f,0.0f,  dimf,-dimf, dimf );

        *it++ = Vertex( 0.0f,0.0f, -dimf,-dimf,-dimf );
        *it++ = Vertex( 1.0f,0.0f, -dimf,-dimf, dimf );
        *it++ = Vertex( 1.0f,1.0f, -dimf, dimf, dimf );
        *it   = Vertex( 0.0f,1.0f, -dimf, dimf,-dimf );

    }

    ~RigidBodyRenderer()
    {
        free(m_cubeVertices);
    }

    void render()
    {
        static unsigned int s_frameNumber = 0;
        //perform next simulation step
        m_rigidBodySimulation.update(++s_frameNumber);

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        //render the "spring" as a line whose thickness varies 
        //depending on the tension/length
        const RigidBodySimulation::Vec3& springEnd1 = m_rigidBodySimulation.getSpringEndPoint1();
        //convert as a position, therefore the last argument is true.
        RigidBodySimulation::Vec3 springEnd2;
        m_rigidBodySimulation.getSpringEndPoint2(m_currentEquation,springEnd2);

        RigidBodySimulation::Vec3 springVec = springEnd2 - springEnd1;
        double length = springVec.normalize();
        float lineWidth = 1.0;
        if(length > m_rigidBodySimulation.getSpringForce().getUnstretchedLength())
        {
            lineWidth = 3.0;
        }
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        //glLoadIdentity();
        glLineWidth(lineWidth);
        glBegin(GL_LINES);
        //change the spring color depending on which equation we visualize
        if(m_currentEquation == RigidBodySimulation::RIGID_BODY_EQUATION_WRF)
            glColor3f(1,0,0);
        else 
            glColor3f(0,0,1);
        glVertex3d(springEnd1[0],springEnd1[1],springEnd1[2]);
        glVertex3d(springEnd2[0],springEnd2[1],springEnd2[2]);
        glEnd();
        glLineWidth(1.0);
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);

        //add a subtle color variation depending on the equation type
        //that is being used for rendering
        if(m_currentEquation == RigidBodySimulation::RIGID_BODY_EQUATION_WRF)
            glColor3f(1.0f,0.85f,0.85f);
        else 
            glColor3f(0.85f,0.85f,1.0f);

        //modelview transform
        GLfloat matrix[4][4];

        //*****************************************
        //Here we truncate from double to float!
        //Rigid body is integrated in doubles
        //*****************************************
        const RigidBodySimulation::Vec3& pos = 
              m_rigidBodySimulation.getRigidBodyPosition(m_currentEquation);
        //position first
        matrix[3][0] = (float)pos[0];
        matrix[3][1] = (float)pos[1];
        matrix[3][2] = (float)pos[2];
        matrix[3][3] = 1.0f;
        matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0f;

        //orientation
        const RigidBodySimulation::Matrix& mat = m_rigidBodySimulation.getRigidBodyOrientation(m_currentEquation);
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                matrix[i][j] = (float)mat[j][i];

        glLoadMatrixf((GLfloat*)&matrix[0][0]);

        //
        // Use the texture's alpha channel to blend it with whatever’s already 
        // in the frame-buffer.
        //

        glDisable( GL_DEPTH_TEST );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glBindTexture( GL_TEXTURE_2D, g_textureID );

        //
        // Use the cull-mode sorting trick for convex non-overlapping 
        // geometry.
        //

        glEnable( GL_CULL_FACE );

        //
        // Render the cube but only render the back-facing polygons.
        //

        glCullFace( GL_FRONT );

        glInterleavedArrays( GL_T2F_V3F, 0, m_cubeVertices );
        glDrawArrays( GL_QUADS, 0, 24 );

        //
        // Render the cube again, but this time we only render the 
        // front-facing polygons.
        //

        glCullFace( GL_BACK );

        glInterleavedArrays( GL_T2F_V3F, 0, m_cubeVertices );
        glDrawArrays( GL_QUADS, 0, 24 );

        glDisable( GL_CULL_FACE );

        SwapBuffers( g_hDC );

    }

    void onMouseInput(long dx, long dy)
    {
        //normalize the mouse move
        double ddx = dx/5.0;
        double ddy = -dy/5.0;
        RigidBodySimulation::Vec3 springEnd1 = m_rigidBodySimulation.getSpringEndPoint1();
        springEnd1.set(springEnd1[0]+ddx,springEnd1[1]+ddy,springEnd1[2]);
        m_rigidBodySimulation.setSpringEndPoint1(springEnd1);
    }

    void onKeyInput(char key )
    {
        switch(key) {
        case ' ':
            m_rigidBodySimulation.resetSimulation(); 
            break;
        case '1':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(dim/2.0,dim/2.0,dim/2.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case '2':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(-dim/2.0,dim/2.0,dim/2.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case '3':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(dim/2.0,dim/2.0,-dim/2.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case '4':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(-dim/2.0,dim/2.0,-dim/2.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case '5':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(0.0,dim/2.0,0.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case '6':
            {
                RigidBodySimulation::Vec3 point;
                double dim = m_rigidBodySimulation.getRigidBodyDimension(m_currentEquation);
                point.set(dim/4.0,dim/2.0,3*dim/8.0);
                m_rigidBodySimulation.setSpringAttachmentPointBody(point); 
            }
            break;
        case 't':
        case 'T':
            {
                //toggle between WRF and BRF rigid body equations
                m_currentEquation = (m_currentEquation == RigidBodySimulation::RIGID_BODY_EQUATION_WRF) ?
                    RigidBodySimulation::RIGID_BODY_EQUATION_BRF :
                RigidBodySimulation::RIGID_BODY_EQUATION_WRF;
            }
            break;

        case 'k':
            {
                double springStiffness = m_rigidBodySimulation.getSpringForce().getStiffness()-0.05;
                m_rigidBodySimulation.getSpringForce().setStiffness(springStiffness);
                printf("Spring stiffness %g\n",m_rigidBodySimulation.getSpringForce().getStiffness());
            }
            break;
        case 'K':
            {
                double springStiffness = m_rigidBodySimulation.getSpringForce().getStiffness()+0.05;
                m_rigidBodySimulation.getSpringForce().setStiffness(springStiffness);
                printf("Spring stiffness %g\n",m_rigidBodySimulation.getSpringForce().getStiffness());
            }
            break;
        case 'd':
            {
                double damping = m_rigidBodySimulation.getDampingTorque().getDamping()-0.1;
                m_rigidBodySimulation.getDampingTorque().setDamping(damping);
                printf("Omega damping %g\n",m_rigidBodySimulation.getDampingTorque().getDamping());
            }
            break;
        case 'D':
            {
                double damping = m_rigidBodySimulation.getDampingTorque().getDamping()+0.1;
                m_rigidBodySimulation.getDampingTorque().setDamping(damping);
                printf("Omega damping %g\n",m_rigidBodySimulation.getDampingTorque().getDamping());
            }
        break;
        default:
            break;
        }
    }

    Vertex*  m_cubeVertices;

    //see RigidBodySimulation.h for more details
    RigidBodySimulation m_rigidBodySimulation;
    //Defines the results of which equation (either WRF or BRF) is being rendered
    //See RigidBodyEquationWRF.h for more details
    RigidBodySimulation::RigidBodyEquation m_currentEquation;

};

RigidBodyRenderer g_rigidBodyRenderer;

void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);

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
                             "OpenGL - Rigid Body Dynamics",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 640, NULL, NULL, hInstance, NULL );

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
        case WM_CHAR:
		{
            g_rigidBodyRenderer.onKeyInput(wParam);
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
                g_rigidBodyRenderer.onMouseInput((ptCurrentMousePosit.x - ptLastMousePosit.x),
                                                (ptCurrentMousePosit.y - ptLastMousePosit.y));
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
            glFrustum(-100.0f, 100.0f, -100.0f, 100.0f, 320.0f, 640.0f);
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
    tgaImageFile tgaImage;
    tgaImage.load( "radiation_box.tga" );

    glGenTextures( 1, &g_textureID );

    glBindTexture( GL_TEXTURE_2D, g_textureID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
                  tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, 
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

	loadTexture();

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
	glEnable( GL_TEXTURE_2D );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
    glFrustum(-100.0f, 100.0f, -100.0f, 100.0f, 320.0f, 640.0f);
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
    g_rigidBodyRenderer.render();
}
