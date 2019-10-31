#include <windows.h>		
#include <stdio.h>			
#include <gl\gl.h>			
#include <gl\glu.h>			
#include <gl\glaux.h>		


#include "OpenGLExtensions.h"	//From www.humus.ca (Big thanks to him)
#include "VertexAnimation.h"

extern HDC  hDC;
extern HWND	hWnd;

bool	keys[256];			


GLfloat LightAmbient[]=		{ 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]=		{ 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[]=	{ 0.0f, 0.0f, 100.0f, 1.0f };
//Here we'll hol all the meshes
std::vector<CVertexAnimation*> cMeshes;
uint32						   nCurMesh  = 0;
float						   fRotAngle = 0;
float						   fDistance = 100.0f;




int InitGL(GLvoid)										
{
	initExtensions(hDC);
	//Disable vertical sync for actual fps display
	if(wglSwapIntervalEXT)wglSwapIntervalEXT(0);

	glShadeModel(GL_SMOOTH);							
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				
	glClearDepth(1.0f);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);	
	glEnable(GL_LIGHT1);								
	glEnable(GL_LIGHTING);


	cMeshes.push_back(new CVertexAnimation("Data\\box-anim.MSA"));
	cMeshes.push_back(new CVertexAnimation("Data\\sphere-anim.MSA"));
	cMeshes.push_back(new CVertexAnimation("Data\\plane-anim.MSA"));
	cMeshes.push_back(new CVertexAnimation("Data\\sphere-cube-anim.MSA"));
	
	MessageBox(hWnd,"Controls:\nCursor keys - camera\nN - switch mesh\nHold 'V' - to draw without vbos\nHold 'U' - for not to update","Controls",MB_OK);

	//Check is opengl version is >1.5
	if(GL_1_5_supported==false)MessageBox(hWnd,"Your video card doesn't support vbo's,\nbut you can still view vertex animation\nwithout vbo's","Information",MB_OK);

	return TRUE;										
}

int DeInitGL(GLvoid)
{
	for(uint32 i=0;i<cMeshes.size();i++)
		delete cMeshes[i];
	cMeshes.clear();
	return TRUE;
}


int DrawGLScene(GLvoid)									
{	
	//Calculate fDeltaTime, so the animation would progress with the same speed on different computers
	static DWORD nLastTime = GetTickCount();
	static DWORD nFPSTime  = GetTickCount();
	static int   nFrameCnt = 0;
	DWORD		 nCurTime  = GetTickCount();
	
	//Simple code for fps calculation
	if(nCurTime-nFPSTime>1000)
	{	
		char szNewWndText[300];
		nFPSTime = nCurTime;
		sprintf(szNewWndText,"Vertex Animation using VBO - FPS : %d Vertices : %d",nFrameCnt,cMeshes[nCurMesh]->GetVertexCount());
		SetWindowText(hWnd,szNewWndText);		
		nFrameCnt = 0;
	}
	else nFrameCnt++;

	//Always supply your application with delta time
	float		 fDeltaTime = float(nCurTime-nLastTime)/1000.0f;
	nLastTime = nCurTime;

	//Just some control updating
	if(keys['N'])nCurMesh = (nCurMesh+1)%cMeshes.size();keys['N'] = false;
	if(keys[VK_LEFT]) fRotAngle += fDeltaTime*40.0f;
	if(keys[VK_RIGHT]) fRotAngle -= fDeltaTime*40.0f;
	if(keys[VK_UP]) fDistance -= fDeltaTime*100.0f;
	if(keys[VK_DOWN]) fDistance += fDeltaTime*100.0f;
	//Keep rot angle in [0,360]
	if(fRotAngle<0.0f)fRotAngle+=360.0f;
	if(fRotAngle>360.0f)fRotAngle-=360.0f;



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();	
	gluLookAt(fDistance,fDistance,fDistance,
			  0.0f,0.0f,0.0f,
			  0.0f,0.0f,1.0f);
	glRotatef(fRotAngle,0.0f,0.0f,1.0f);

	if(keys['U']==false)cMeshes[nCurMesh]->Update(fDeltaTime);
	cMeshes[nCurMesh]->Render(keys['V']==false);
	

	return TRUE;										
}