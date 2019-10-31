#include "CreateGL.h"

/* 
This class basically handles all the openGL stuff necessary for implementing our light map
via projecting a texture. This technique handles all the texture matrix processing, lighting, and material
via the multitexturing and the fixed function pipeline
*/

PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = NULL;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;

float CreateGL::rotateObjectX = 0.0f;
float CreateGL::rotateObjectY = 0.0f;
float CreateGL::rotateTextureX = 0.0f;
float CreateGL::rotateTextureY = 0.0f;
float CreateGL::moveEyePoint = 0.0f;

bool CreateGL::objectMove = false;
bool CreateGL::textureMove = false;

VertexData cube[] = 
{
	{1.0,0.0,	1.0,1.0,1.0,1.0,	0.0,0.0,1.0,	-0.5,-0.5,0.5},
	{0.0,0.0,	1.0,1.0,1.0,1.0,	0.0,0.0,1.0,	0.5,-0.5,0.5},
	{0.0,1.0,	1.0,1.0,1.0,1.0,	0.0,0.0,1.0,	0.5,0.5,0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	0.0,0.0,1.0,	-0.5,0.5,0.5},

	{1.0,0.0,	1.0,1.0,1.0,1.0,	1.0,0.0,0.0,	0.5,-0.5,0.5},
	{0.0,0.0,	1.0,1.0,1.0,1.0,	1.0,0.0,0.0,	0.5,-0.5,-0.5},
	{0.0,1.0,	1.0,1.0,1.0,1.0,	1.0,0.0,0.0,	0.5,0.5,-0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	1.0,0.0,0.0,	0.5,0.5,0.5},

	{0.0,0.0,	1.0,1.0,1.0,1.0,	0.0,0.0,-1.0,	-0.5,-0.5,-0.5},
	{0.0,1.0,	1.0,1.0,1.0,1.0,	0.0,0.0,-1.0,	-0.5,0.5,-0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	0.0,0.0,-1.0,	0.5,0.5,-0.5},
	{1.0,0.0,	1.0,1.0,1.0,1.0,	0.0,0.0,-1.0,	0.5,-0.5,-0.5},

	{1.0,0.0,	1.0,1.0,1.0,1.0,	-1.0,0.0,0.0,	-0.5,-0.5,-0.5},
	{0.0,0.0,	1.0,1.0,1.0,1.0,	-1.0,0.0,0.0,	-0.5,-0.5,0.5},
	{0.0,1.0,	1.0,1.0,1.0,1.0,	-1.0,0.0,0.0,	-0.5,0.5,0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	-1.0,0.0,0.0,	-0.5,0.5,-0.5},

	{0.0,1.0,	1.0,1.0,1.0,1.0,	0.0,-1.0,0.0,	-0.5,-0.5,-0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	0.0,-1.0,0.0,	0.5,-0.5,-0.5},
	{1.0,0.0,	1.0,1.0,1.0,1.0,	0.0,-1.0,0.0,	0.5,-0.5,0.5},
	{0.0,0.0,	1.0,1.0,1.0,1.0,	0.0,-1.0,0.0,	-0.5,-0.5,0.5},

	{0.0,0.0,	1.0,1.0,1.0,1.0,	0.0,1.0,0.0,	-0.5,0.5,-0.5},
	{0.0,1.0,	1.0,1.0,1.0,1.0,	0.0,1.0,0.0,	-0.5,0.5,0.5},
	{1.0,1.0,	1.0,1.0,1.0,1.0,	0.0,1.0,0.0,	0.5,0.5,0.5},
	{1.0,0.0,	1.0,1.0,1.0,1.0,	0.0,1.0,0.0,	0.5,0.5,-0.5},
};

CreateGL::CreateGL()
{
	TexObj = new TexProj;
}

CreateGL::~CreateGL()
{
	delete TexObj;
}

void CreateGL::InitGL()
{
	GLuint pixelformat;
	PIXELFORMATDESCRIPTOR pfd;
	GLfloat light0Pos[4] =  {0.0f, 0.0f, 0.2f, 1.0f};
	GLfloat matAmb[4] =  {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat matDiff[4] =  {0.85f, 0.85f, 0.85f, 1.0f};
	GLfloat matSpec[4] =  {0.30f, 0.30f, 0.30f, 1.0f};
	GLfloat matShine = 35.0;

	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.nVersion = 1;

	hDC = GetDC(hWnd);
	pixelformat = ChoosePixelFormat(hDC,&pfd);
	SetPixelFormat(hDC,pixelformat,&pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);

	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");

	//Make sure multitexturing is supported
	if(!glMultiTexCoord2fARB && !glActiveTextureARB)
	{
		MessageBox(NULL,"MultiTexturing is not supported by your hardware","ERROR",MB_OK);
		return;
	}

	//Enable front face culling so we can see into the box
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glClearColor(0.0f,0.0f,0.0f,1.0f);

	//apply color tracking for material properties
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matSpec);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,matShine);

	//set up lighting so we have a dark scene
	glLightfv(GL_LIGHT0,GL_POSITION,light0Pos);
	glLightfv(GL_LIGHT0,GL_AMBIENT, matAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, matDiff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, matSpec);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.5f,0.5,-0.5f,0.5f,1.0f,15.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,1024,768);
	gluLookAt(0.0,0.0,2.0,0.0,0.0,0.0,0.0,1.0,0.0);

	//activate and load base texture on stage_0
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	TexObj->loadTexture("stone_wall.bmp");

	//activate and load lightmap on stage_1
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	TexObj->loadLightMapTexture("lightmap.bmp");
}


void CreateGL::KillGL()
{
	if(hRC != NULL){
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(hRC);
		hRC = NULL;
	}
	if(hDC != NULL){
		ReleaseDC(hWnd,hDC);
		hDC = NULL;
	}
}

void CreateGL::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//control the movement of the projected texture
	glPushMatrix();
	glLoadIdentity();
	glRotatef(rotateTextureX - rotateObjectX,1.0f,0.0f,0.0f);
	glRotatef(rotateTextureY - rotateObjectY,0.0f,1.0f,0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX,MVmatrix);
	TextureTransform.setMatrix(MVmatrix);
	glPopMatrix();
	TexObj->textureProjection(TextureTransform);

	if(objectMove)
	{
		//move the box itself
		float objmatrix[16];
		glPushMatrix();
		glLoadIdentity();
		glRotatef(rotateObjectX,-1.0f,0.0f,0.0f);
		glRotatef(rotateObjectY,0.0f,-1.0f,0.0f);
		glGetFloatv(GL_MODELVIEW_MATRIX,objmatrix);
		ObjectTransform.setMatrix(objmatrix);
		glPopMatrix();
	}

	//apply transformation on box and draw it
	glPushMatrix();
	glMultMatrixf(ObjectTransform.getMatrix());
	glInterleavedArrays(GL_T2F_C4F_N3F_V3F,0,cube);
	glDrawArrays(GL_QUADS,0,24);
	glPopMatrix();

	SwapBuffers(hDC);
}

LRESULT CALLBACK CreateGL::WndProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINT lastPoint;
	static POINT currentPoint;
	int width, height;

	switch(message){
		case WM_KEYDOWN:
			switch(wParam){
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_UP:
					moveEyePoint += 0.02f;
					break;

				case VK_DOWN:
					moveEyePoint -= 0.02f;
					break;
			}
			break;

			//controls motion of box
		case WM_LBUTTONDOWN:
			lastPoint.x = currentPoint.x = HIWORD(lParam);
			lastPoint.y = currentPoint.y = LOWORD(lParam);
			objectMove = true;
			break;

			//controls motion of projected texture
		case WM_RBUTTONDOWN:
			lastPoint.x = currentPoint.x = HIWORD(lParam);
			lastPoint.y = currentPoint.y = LOWORD(lParam);
			textureMove = true;
			break;

		case WM_RBUTTONUP:
			textureMove = false;
			break;

		case WM_LBUTTONUP:
			objectMove = false;
			break;

		case WM_MOUSEMOVE:
			currentPoint.x = HIWORD(lParam);
			currentPoint.y = LOWORD(lParam);

			if(objectMove){
				rotateObjectX -= currentPoint.x - lastPoint.x;
				rotateObjectY -= currentPoint.y - lastPoint.y;
			}
			if(textureMove){
				rotateTextureX -= currentPoint.x - lastPoint.x;
				rotateTextureY -= currentPoint.y - lastPoint.y;
			}
			lastPoint = currentPoint;
			break;

		case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			glViewport(0,0,width,height);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f,(double)width/(double)height,0.1f,100.0f);
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWindow,message,wParam,lParam);
			break;
	}
	return 0;
}