#include "Radar.h"

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC    wglCreatePbufferARB    = NULL;
PFNWGLGETPBUFFERDCARBPROC     wglGetPbufferDCARB     = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC   wglDestroyPbufferARB   = NULL;
PFNWGLQUERYPBUFFERARBPROC     wglQueryPbufferARB     = NULL;

// WGL_ARB_pixel_format
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

float translate, rotateX, rotateY;
bool NVGeffect;

VertexData Floor[] = { {0.0f, 0.0f,		-20.0f, -5.0f, 20.0f},
						{5.0f, 0.0f,	20.0f, -5.0f, 20.0f},		//floor
						{5.0f, 5.0f,	20.0f, -5.0f, -20.0f},
						{0.0f, 5.0f,	-20.0f, -5.0f, -20.0f} };

VertexData Walls[] = { {0.0f, 0.0f,		-20.0f, -5.0f, -20.0f},
						{5.0f, 0.0f,	20.0f, -5.0f, -20.0f},		//front wall
						{5.0f, 5.0f,	20.0f, 5.0f, -20.0f},
						{0.0f, 5.0f,	-20.0f, 5.0f, -20.0f},

						{0.0f, 0.0f,	20.0f, -5.0f, -20.0f},
						{5.0f, 0.0f,	20.0f, -5.0f, 20.0f},		//right wall
						{5.0f, 5.0f,	20.0f, 5.0f, 20.0f},
						{0.0f, 5.0f,	20.0f, 5.0f, -20.0f},

						{0.0f, 0.0f,	20.0f, -5.0f, 20.0f},
						{5.0f, 0.0f,	-20.0f, -5.0f, 20.0f},		//back wall
						{5.0f, 5.0f,	-20.0f, 5.0f, 20.0f},
						{0.0f, 5.0f,	20.0f, 5.0f, 20.0f},

						{0.0f, 0.0f,	-20.0f, -5.0f, -20.0f},
						{5.0f, 0.0f,	-20.0f, -5.0f, 20.0f},		//left wall
						{5.0f, 5.0f,	-20.0f, 5.0f, 20.0f},
						{0.0f, 5.0f,	-20.0f, 5.0f, -20.0f} };

int Radar::size = 512;

Radar::Radar()
{
	extensions = new char[2048];
	line = gluNewQuadric();
	translate = 0.0f;
	NVGeffect = false;
}

Radar::~Radar()
{
	delete[] extensions;
	gluDeleteQuadric(line);
}

void Radar::InitGL()
{
	GLuint pixelformat;
	PIXELFORMATDESCRIPTOR pfd;
	GLfloat light0Pos[4] =  {0.0, 5.0, 0.0, 1.0};
	GLfloat litAmb[4] = {0.2, 0.2, 0.2, 1.0};
	GLfloat litDiff[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat litAtten = 0.3;
	GLfloat matAmb[4] =  {0.1, 0.1, 0.1, 1.0};
	GLfloat matDiff[4] =  {0.5, 0.5, 0.5, 1.0};

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

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glClearColor(0.0f,0.0f,0.0f,1.0f);

	glLightfv(GL_LIGHT0,GL_POSITION,light0Pos);			//set up light parameters
	glLightfv(GL_LIGHT0,GL_AMBIENT,litAmb);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,litDiff);
	glLightfv(GL_LIGHT0,GL_LINEAR_ATTENUATION,&litAtten);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();						//setup OGL perspective
	gluPerspective(45.0,1.33,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,1024,768);

	makeTexture(size,3);		//create texture that will hold our dynamic texture data for persistance effect

	InitializePBuffer();

	AUX_RGBImageRec *Image = NULL;
	Image = auxRGBImageLoad("bark.rgb");		//load and bind any old texture to have 3D geometry 
	glGenTextures(1,&barktex);					//underneath overlay
	glBindTexture(GL_TEXTURE_2D,barktex);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Image->sizeX,Image->sizeY,0,GL_RGB,GL_UNSIGNED_BYTE,Image->data);

	if(Image)
	{
		if(Image->data)
			free(Image->data);
		free(Image);
	}

	AUX_RGBImageRec *Image2 = NULL;
	Image2 = auxRGBImageLoad("floor5.rgb");		//load and bind any old texture to have 3D geometry 
	glGenTextures(1,&floortex);					//underneath overlay
	glBindTexture(GL_TEXTURE_2D,floortex);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Image2->sizeX,Image2->sizeY,0,GL_RGB,GL_UNSIGNED_BYTE,Image2->data);

	if(Image2)
	{
		if(Image2->data)
			free(Image2->data);
		free(Image2);
	}
}

void Radar::InitializePBuffer()
{

	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");

	if(wglGetExtensionsStringARB)
		strcpy(extensions, wglGetExtensionsStringARB(wglGetCurrentDC()));
	else{
		MessageBox(NULL,"Could not get wgl extensions","ERROR",MB_OK);
		return;
	}

	//verify if we have pbuffer extension on current hardware
	if(strstr(extensions,"WGL_ARB_pbuffer") == NULL){
		MessageBox(NULL,"pbuffer is not supported","ERROR",MB_OK);
		return;
	}
	else{
		wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC) wglGetProcAddress("wglCreatePbufferARB");
		wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC) wglGetProcAddress("wglGetPbufferDCARB");
		wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC) wglGetProcAddress("wglReleasePbufferDCARB");
		wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC) wglGetProcAddress("wglDestroyPbufferARB");
		wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC) wglGetProcAddress("wglQueryPbufferARB");
	}

	if(!wglCreatePbufferARB || !wglGetPbufferDCARB || !wglReleasePbufferDCARB || !wglDestroyPbufferARB ||
		!wglQueryPbufferARB)
	{
		MessageBox(NULL,"Could not find one or all function pointers for pbuffer","ERROR",MB_OK);
		return;
	}

	//same test for pixel format
	if(strstr(extensions,"WGL_ARB_pixel_format") == NULL)
	{
		MessageBox(NULL,"Could not get wgl pixel format","ERROR",MB_OK);
		return;
	}
	else{
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
	}

	if(!wglChoosePixelFormatARB){
		MessageBox(NULL,"Could not get a valid function for pixel format","ERROR",MB_OK);
		return;
	}

	pbuffer.hPbuffer = NULL;
	pbuffer.width = size;
	pbuffer.height = size;

	//setup attributes to aquire pixel format for pbuffer down below
	int attr[] = 
	{
		WGL_SUPPORT_OPENGL_ARB, TRUE,
		WGL_DRAW_TO_PBUFFER_ARB, TRUE,
		WGL_BIND_TO_TEXTURE_RGBA_ARB,TRUE,
		WGL_RED_BITS_ARB,8,
		WGL_BLUE_BITS_ARB,8,
		WGL_GREEN_BITS_ARB,8,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,16,
		WGL_DOUBLE_BUFFER_ARB,FALSE,
		0
	};

	unsigned int count = 0;
	int format;
	//try to get pixel format for pbuffer
	wglChoosePixelFormatARB(hDC,attr,NULL,1,&format,&count);

	if(count == 0)
	{
		MessageBox(NULL,"Could not get a valid pixel format","ERROR",MB_OK);
		return;
	}

	//set up pbuffer attributes
	int pb_attr[] = 
	{
		WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,
		WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
		0
	};

	pbuffer.hPbuffer = wglCreatePbufferARB(hDC,format,pbuffer.width,pbuffer.height,pb_attr);
	pbuffer.hDC = wglGetPbufferDCARB(pbuffer.hPbuffer);
	pbuffer.hRC = wglCreateContext(pbuffer.hDC);

	if(!pbuffer.hPbuffer)
	{
		MessageBox(NULL,"Could not create pbuffer context","ERROR",MB_OK);
		return;
	}

	int h;
	int w;
	wglQueryPbufferARB(pbuffer.hPbuffer,WGL_PBUFFER_WIDTH_ARB,&w);
	wglQueryPbufferARB(pbuffer.hPbuffer,WGL_PBUFFER_HEIGHT_ARB,&h);
	
	//make sure everything we have setup and is correct within pbuffer 
	if((w != pbuffer.width) && (h != pbuffer.height))
	{
		MessageBox(NULL,"widths and heights of pbuffer are not matching","ERROR",MB_OK);
		return;
	}

	//share display lists between frame buffer and pbuffer
	if(!wglShareLists(hRC,pbuffer.hRC))
	{
		MessageBox(NULL,"Could not share display lists","ERROR",MB_OK);
		return;
	}

	wglMakeCurrent(pbuffer.hDC,pbuffer.hRC);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);		//set up pbuffer perspective
	glLoadIdentity();
	gluOrtho2D(0.0,1.0,0.0,1.0);
	glViewport(0,0,size,size);
}

void Radar::KillGL()
{
	if(hRC != NULL){
		wglMakeCurrent(NULL,NULL);		//release OGL rendering context
		wglDeleteContext(hRC);
		hRC = NULL;
	}
	if(hDC != NULL){
		ReleaseDC(hWnd,hDC);
		hDC = NULL;
	}

	if(pbuffer.hRC != NULL)
	{
		wglMakeCurrent(pbuffer.hDC,pbuffer.hRC);			//release pbuffer rendering context
		wglDeleteContext(pbuffer.hRC);
		wglReleasePbufferDCARB(pbuffer.hPbuffer,pbuffer.hDC);
		wglDestroyPbufferARB(pbuffer.hPbuffer);
		pbuffer.hRC = NULL;
	}
	if(pbuffer.hDC != NULL)
	{
		ReleaseDC(hWnd,pbuffer.hDC);
		pbuffer.hDC = NULL;
	}
}

void Radar::makeTexture(int dimension, int channels)
{

	glGenTextures(1,&texId);				//create texture that will store data for persistance
	glBindTexture(GL_TEXTURE_2D,texId);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,4,dimension,dimension,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
}

DWORD Radar::FrameTimeDifference(DWORD time)
{
	DWORD currenttime;

	currenttime = timeGetTime();

	return (currenttime - time);
}

void Radar::DrawBackground()
{
	//only enable green channel to give a pseudo radar looking env
	if(NVGeffect)
		glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);			//set up states so we can see geometry in the scene
	glBindTexture(GL_TEXTURE_2D,floortex);
	glColor3f(1.0f,1.0f,1.0f);				//take away the green channel here and the geometry will not be seen
											//because we are only allowing green channel to be visible 

	glInterleavedArrays( GL_T2F_V3F, 0, Floor );	//draw floor
    glDrawArrays( GL_QUADS, 0, 4 );

	glBindTexture(GL_TEXTURE_2D,barktex);

	glInterleavedArrays( GL_T2F_V3F, 0, Walls );	//draw walls
	glDrawArrays( GL_QUADS, 0, 16 );
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
	
	if(NVGeffect)
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);	//have to reset color mask otherwise we get wierd artifacts
}

void Radar::DrawOverlay(double increment)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(rotateY,0.0f,-1.0f,0.0f);		//give some motion capabilities within the scene
	glRotatef(rotateX,1.0f,0.0f,0.0f);
	glTranslatef(0.0f,0.0f,-6.0f - translate);		
	DrawBackground();					//draw stuff out in the scene

	glBindTexture(GL_TEXTURE_2D,texId);	//bind our overlay texture 
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();				//push matrices so that we can go to orthographic mode to do overlay
		
		glDisable(GL_DEPTH_TEST);		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);		//enable blending to allow transparency between radar sweep
												//and the rest of the scene
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0.0,1.0,0.0,1.0);
			glMatrixMode(GL_MODELVIEW);

			glPushAttrib(GL_LIGHTING_BIT);		//push lighting state so we can change it. could have simply called
												//glDisable(GL_LIGHTING) directly and turn it back on later.
			glDisable(GL_LIGHTING);				//disable lighting for overlay

			glColor4f(0.0f,1.0f,0.0f,0.96f);	//change alpha value to either shorten or lengthen radar ring bands
			glLoadIdentity();
			glBegin(GL_QUADS);
				glTexCoord2f(0.0,0.0);	glVertex2f(0.0,0.0);
				glTexCoord2f(1.0,0.0);	glVertex2f(1.0,0.0);		//draw overlay with radar sweep
				glTexCoord2f(1.0,1.0);	glVertex2f(1.0,1.0);
				glTexCoord2f(0.0,1.0);	glVertex2f(0.0,1.0);
			glEnd();
			glEnable(GL_DEPTH_TEST);	//make these state changes so that we can render the radar leading edge dots
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);

			glColor3f(0.0f,1.0f,0.0f);
			glTranslatef(0.5f,0.5f,0.0f);			//translate to center of screen

			glRotatef(increment,0.0f,0.0f,-1.0f);
			
			glTranslatef(0.1f,0.0f,0.0f);
			gluDisk(line,0.0,0.005,16,16);		//render each leading dot of radar ring

			glTranslatef(0.1f,0.0f,0.0f);
			gluDisk(line,0.0,0.005,16,16);

			glTranslatef(0.1f,0.0f,0.0f);
			gluDisk(line,0.0,0.005,16,16);

			glPopAttrib();					//restore lighting
			glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);			//restore OGL matrices
	glPopMatrix();
}

void Radar::Draw()
{
	static double iterate = 0.05;	//used to control the sweep of the radar rings
	static bool first = true;		
	int lost = 0;
	wglQueryPbufferARB(pbuffer.hPbuffer,WGL_PBUFFER_LOST_ARB,&lost);
	if(lost != 0)
	{
		MessageBox(NULL,"pbuffer lost","ERROR",MB_OK);
		return;
	}

	if(first){
		lasttime = timeGetTime();	//initialize timing variables
		first = false;
	}
	
	if(FrameTimeDifference(lasttime) > 33)		//only get data from pbuffer if last time was over 33 ms
	{
		//in here is where the persistance occurs. Each time the overlay gets drawn into the pbuffer, 
		//an alpha channel value 0.96 gets multiplied each time it gets rendered. This means the first pass will
		//have an alpha value of 0.96, the second pass will contain what happened from the first pass plus the 
		//new updated position of the radar sweep.
		//The second pass applies an alpha value of 0.96 to that and as you can see, each pass
		//keeps on multiplying by 0.96 which gives the persistance you see in the rings. 
		//Essentially each time the new position of the rings gets captured, with the glCopyTexSubImage. The overlay 
		//has an alpha value of 0.96, which means that each previous time the value will be multiplied by 0.96.
		//That is the recursive nature that produces the persistance.
		lasttime = timeGetTime();

		wglMakeCurrent(pbuffer.hDC,pbuffer.hRC);	//make pbuffer current to draw into
		DrawOverlay(iterate);
		glBindTexture(GL_TEXTURE_2D,texId);
		glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,pbuffer.width,pbuffer.height);	//copy pbuffer contents 
	}

	wglMakeCurrent(hDC,hRC);		//set up so that we draw into our normal frame buffer
	DrawOverlay(iterate);

	if(iterate >= 360.0)
		iterate = 0.0;
	else
		iterate += 0.05;

	SwapBuffers(hDC);
}

LRESULT CALLBACK Radar::WndProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINT Rotate, lastRotate;
	static float move;
	static bool mouse;

	switch(message){
		case WM_KEYDOWN:
			switch(wParam){
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_UP:
					translate -= 0.5f;
					break;
				case VK_DOWN:
					translate += 0.5f;
					break;

				case VK_F1:
					NVGeffect = !NVGeffect;
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			mouse = true;
			Rotate.x = lastRotate.x = LOWORD(lParam);
			Rotate.y = lastRotate.y = HIWORD(lParam);
			break;

		case WM_LBUTTONUP:
			mouse = false;
			break;

		case WM_MOUSEMOVE:
			Rotate.x = LOWORD(lParam);
			Rotate.y = HIWORD(lParam);

			if(mouse)
			{
				rotateY -= (Rotate.x - lastRotate.x)/2;
				rotateX -= (Rotate.y - lastRotate.y)/2;
			}

			lastRotate = Rotate;
			break;

		case WM_SIZE:
		{
			int width  = LOWORD(lParam); 
			int height = HIWORD(lParam);
			glViewport(0, 0, width, height);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)width / (GLdouble)height, 0.1, 100.0);
		}
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