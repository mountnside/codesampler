//-----------------------------------------------------------------------------
//           Name: sdl_ogl_gpu_zfail.cpp
//         Author: Andreas T Jonsson
//  Last Modified: 2006
//    Description: This sample demonstrates how to render z-fail shadows with
//                 the GPU and how to take advantage of the two side stencil extension.
//-----------------------------------------------------------------------------

//Befor you start with this tutorial you should have good knowledge of GLSL
//and the z-fail algorythem. You can find tutorials on those subjects at
//http://www.codesampler.com. You should also read this article
//(http://www.gamedev.net/reference/articles/article1990.asp) befor you begin.

#include <iostream>

#ifdef WIN32
	#include <Windows.h>
	#include <SDL.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#include <gl/wglext.h>
	#include <gl/glext.h>
#else
	#include <SDL/SDL.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glx.h>
	#include <GL/glext.h>
#endif

#include "geometry.h"


//-----------------------------------------------------------------------------

PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgramObjectARB	= NULL;
PFNGLCREATESHADEROBJECTARBPROC		glCreateShaderObjectARB		= NULL;
PFNGLLINKPROGRAMARBPROC				glLinkProgramARB			= NULL;
PFNGLCOMPILESHADERARBPROC			glCompileShaderARB			= NULL;
PFNGLGETINFOLOGARBPROC				glGetInfoLogARB				= NULL;
PFNGLDELETEOBJECTARBPROC			glDeleteObjectARB			= NULL;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgramObjectARB		= NULL;
PFNGLSHADERSOURCEARBPROC			glShaderSourceARB			= NULL;
PFNGLATTACHOBJECTARBPROC			glAttachObjectARB			= NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameterivARB	= NULL;
PFNGLGETUNIFORMLOCATIONARBPROC		glGetUniformLocationARB		= NULL;
PFNGLGETATTRIBLOCATIONARBPROC		glGetAttribLocationARB		= NULL;
PFNGLUNIFORM4FARBPROC				glUniform4fARB				= NULL;
PFNGLUNIFORM3FARBPROC				glUniform3fARB				= NULL;
PFNGLUNIFORM2FARBPROC				glUniform2fARB				= NULL;
PFNGLUNIFORM1FARBPROC				glUniform1fARB				= NULL;
PFNGLUNIFORM1IARBPROC				glUniform1iARB				= NULL;

//Two side stencil buffer extension.
PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT = NULL;

//-----------------------------------------------------------------------------

struct V3D
{
	float x, y, z;
};

//-----------------------------------------------------------------------------

bool g_bLeftMouse 			= false;
bool g_bShowShadowVolume 	= false;

int g_iLastMouseX = 0;
int g_iLastMouseY = 0;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

float g_fZoom = -6.0f;
float g_pLightPos[] = {1.2f, 1.5f, 1.6f};

GLhandleARB m_glslVertexProgram;
GLhandleARB m_glslFragmentProgram;
GLhandleARB m_glslContext;
unsigned int m_glslLightPos;

//-----------------------------------------------------------------------------

//Models.
V3D g_pBoxVertices[] = {{0.0f, 0.0f, 1.0f}, 	{1.0f, 0.0f, 1.0f}, 	{1.0f, 1.0f, 1.0f}, 	{0.0f, 1.0f, 1.0f},
						{0.0f, 0.0f, 0.0f}, 	{0.0f, 1.0f, 0.0f}, 	{1.0f, 1.0f, 0.0f}, 	{1.0f, 0.0f, 0.0f},
						{0.0f, 1.0f, 0.0f}, 	{0.0f, 1.0f, 1.0f}, 	{1.0f, 1.0f, 1.0f}, 	{1.0f, 1.0f, 0.0f},
						{0.0f, 0.0f, 0.0f}, 	{1.0f, 0.0f, 0.0f}, 	{1.0f, 0.0f, 1.0f}, 	{0.0f, 0.0f, 1.0f},
						{1.0f, 0.0f, 0.0f}, 	{1.0f, 1.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f}, 	{1.0f, 0.0f, 1.0f},
						{0.0f, 0.0f, 0.0f}, 	{0.0f, 0.0f, 1.0f}, 	{0.0f, 1.0f, 1.0f}, 	{0.0f, 1.0f, 0.0f}};

//OBS! This is the face normals!
V3D g_pBoxNormals[] = {{0.0f, 0.0f, 1.0f}, 		{0.0f, 0.0f, 1.0f}, 	{0.0f, 0.0f, 1.0f}, 	{0.0f, 0.0f, 1.0f},
						{0.0f, 0.0f, -1.0f}, 	{0.0f, 0.0f, -1.0f}, 	{0.0f, 0.0f, -1.0f}, 	{0.0f, 0.0f, -1.0f},
						{0.0f, 1.0f, 0.0f}, 	{0.0f, 1.0f, 0.0f}, 	{0.0f, 1.0f, 0.0f}, 	{0.0f, 1.0f, 0.0f},
						{0.0f, -1.0f, 0.0f}, 	{0.0f, -1.0f, 0.0f}, 	{0.0f, -1.0f, 0.0f}, 	{0.0f, -1.0f, 0.0f},
						{1.0f, 0.0f, 0.0f}, 	{1.0f, 0.0f, 0.0f}, 	{1.0f, 0.0f, 0.0f}, 	{1.0f, 0.0f, 0.0f},
						{-1.0f, 0.0f, 0.0f}, 	{-1.0f, 0.0f, 0.0f}, 	{-1.0f, 0.0f, 0.0f}, 	{-1.0f, 0.0f, 0.0f}};

//The thing with the quadrilaterals is very well explained in the article I mentioned.
//This is the extra geometry you need to send to the video card. (One quad for each UNIQE edge.)
V3D g_pQuadrilateralsVertices[] = {{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
									{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
									{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
									{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f},
						
									{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
									{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
									{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f,  1.0f, 0.0f},
									{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
										
									{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
									{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
									
									{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
									{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

//The quadrilaterals normals is per vertex. It shares the normals from its neibours.
V3D g_pQuadrilateralsNormals[] = {{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
									{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
									{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
									{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
									
									{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
									{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
									{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
									{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
									
									{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
									{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
									
									{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
									{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

//-----------------------------------------------------------------------------

char *LoadShader(const char *pFileName)
{
	FILE *pFile = fopen(pFileName, "rb");
	if(!pFile)
		return NULL;

	fseek(pFile, 0, SEEK_END);
	int iSize = ftell(pFile)+1;
	fseek(pFile, 0, SEEK_SET);

	if(iSize < 1)
	{
		fclose(pFile);
		return NULL;
	}

	char *pData = new char[iSize];

	int iBytes = (int)fread(pData, 1, iSize, pFile);
	pData[iBytes] = '\0';
	fclose(pFile);

	return pData;
}

//-----------------------------------------------------------------------------

bool LoadShaders(const char *pVertexProgram, const char *pFragmentProgram)
{
	int iResult;
	char pBuffer[4096];

	//Create program object.
	m_glslContext = glCreateProgramObjectARB();

	//Load vertex program.
	char *pShaderCode = LoadShader(pVertexProgram);
	if(!pShaderCode)
	{
		printf("Can't load %s\n", pVertexProgram);
		return false;
	}
	
	m_glslVertexProgram = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glShaderSourceARB(m_glslVertexProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(m_glslVertexProgram);
	glGetObjectParameterivARB(m_glslVertexProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(m_glslContext, m_glslVertexProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(m_glslVertexProgram, sizeof(pBuffer), NULL, pBuffer);
		printf("Vertex Program Error:\n");
		printf("%s\n", pBuffer);
		return false;
	}

	//Load fragment program.
	pShaderCode = LoadShader(pFragmentProgram);
	if(!pShaderCode)
	{
		printf("Can't load %s\n", pFragmentProgram);
		return false;
	}

	m_glslFragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(m_glslFragmentProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(m_glslFragmentProgram);
	glGetObjectParameterivARB(m_glslFragmentProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(m_glslContext, m_glslFragmentProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(m_glslFragmentProgram, sizeof(pBuffer), NULL, pBuffer);
		printf("Fragment Program Error:\n");
		printf("%s\n", pBuffer);
		return false;
	}

	//Link all the shaders.
	glLinkProgramARB(m_glslContext);
	glGetObjectParameterivARB(m_glslContext, GL_OBJECT_LINK_STATUS_ARB, &iResult);
	if(!iResult)
	{
		glGetInfoLogARB(m_glslContext, sizeof(pBuffer), NULL, pBuffer);
		printf("%s\n", pBuffer);
		return false;
	}
	
	//Get the uniform.
	m_glslLightPos = glGetUniformLocationARB(m_glslContext, "uLightPos");
	
	return true;
}

//-----------------------------------------------------------------------------

bool Initialize()
{
	printf("Initialize GL Extensions and Shaders...\n");
	glCreateProgramObjectARB	= (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
	glCreateShaderObjectARB		= (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
	glCompileShaderARB			= (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
	glLinkProgramARB			= (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
	glGetInfoLogARB				= (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
	glUseProgramObjectARB		= (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
	glShaderSourceARB			= (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
	glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
	glGetObjectParameterivARB	= (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");
	glGetUniformLocationARB		= (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
	glUniform3fARB				= (PFNGLUNIFORM3FARBPROC)SDL_GL_GetProcAddress("glUniform3fARB");
	glActiveStencilFaceEXT 		= (PFNGLACTIVESTENCILFACEEXTPROC)SDL_GL_GetProcAddress("glActiveStencilFaceEXT");

	//Check if extensions fails.
	if(!(glCreateProgramObjectARB && glCreateShaderObjectARB && glCompileShaderARB && glLinkProgramARB &&
		glGetInfoLogARB && glUseProgramObjectARB && glShaderSourceARB && glAttachObjectARB &&
		glGetObjectParameterivARB && glGetUniformLocationARB && glUniform3fARB && glActiveStencilFaceEXT))
	{
		exit(-1);
	}
	
	if(!LoadShaders("./extrude.vp", "./extrude.fp"))
		exit(-1);
	
	//Setup OpenGL.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 800.0f / 600.0f, 1.0f, -1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPointSize(5.0f);
	glClearColor(0.35f, 0.53f, 0.7f, 1.0f);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	
	float pLightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	float pLightDiffuse[] = {1.0, 1.0, 1.0, 1.0}; 
	float pLightSpecular[] = {1.0, 1.0, 1.0, 1.0};
	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, pLightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, pLightSpecular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, pLightAmbient);

	return true;
}

//-----------------------------------------------------------------------------

void RenderModel(V3D *pVertices, V3D *pNormals, int iNumVertices)
{
	glBegin(GL_QUADS);
		for(int i = 0; i < iNumVertices; i++)
		{
			glNormal3fv((float*)&pNormals[i]);
			glVertex3fv((float*)&pVertices[i]);
		}
	glEnd();
}

//-----------------------------------------------------------------------------

void RenderScene()
{
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	
	//Render floor.
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		
		glVertex3f(-2.0f, -1.5f, -2.0f);
		glVertex3f(-2.0f, -1.5f, 2.0f);
		glVertex3f(2.0f, -1.5f, 2.0f);
		glVertex3f(2.0f, -1.5f, -2.0f);
	glEnd();
	
	//Somthing to shadow.
	glPushMatrix();
	glTranslatef(-1.2f, -0.75f, 0.0f);
	glRotatef(55.0f, 0.0f, 1.0f, 0.0f);
	renderSolidTeapot(1.0f);
	glPopMatrix();
	
	//The shadow caster.
	RenderModel(g_pBoxVertices, g_pBoxNormals, (sizeof(g_pBoxVertices) / 4) / 3);
}

//-----------------------------------------------------------------------------

void Render()
{
    if(g_bLeftMouse)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        g_fSpinX -= (x - g_iLastMouseX);
        g_fSpinY -= (y - g_iLastMouseY);

        g_iLastMouseX = x;
        g_iLastMouseY = y;
    }
	
	float pLightPos[] = {g_pLightPos[0], g_pLightPos[1], g_pLightPos[2], 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, pLightPos);
 
	glLoadIdentity();
    glTranslatef(0.0f, 0.0f, g_fZoom);
    glRotatef(-g_fSpinY, 1.0f, 0.0f, 0.0f);
    glRotatef(-g_fSpinX, 0.0f, 1.0f, 0.0f);
	
	//Save all attributes.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//Fill the depth buffer.
	glDisable(GL_LIGHTING);
	RenderScene();
	
	//Prevents z-fighting.
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f, 100.0f);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT); //Use the two side stencil buffer.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	///Incrementing the stencil buffer.
	glActiveStencilFaceEXT(GL_FRONT);
	glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0);

	//Decrement the stencil buffer.
	glActiveStencilFaceEXT(GL_BACK);
	glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0);

	//Render shadow-volume with our shader.
	glUseProgramObjectARB(m_glslContext);
	glUniform3fARB(m_glslLightPos, g_pLightPos[0], g_pLightPos[1], g_pLightPos[2]);
	RenderModel(g_pBoxVertices, g_pBoxNormals, (sizeof(g_pBoxVertices) / 4) / 3);
	RenderModel(g_pQuadrilateralsVertices, g_pQuadrilateralsNormals, (sizeof(g_pQuadrilateralsVertices) / 4) / 3);
	glUseProgramObjectARB((GLhandleARB)NULL);
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthFunc(GL_LEQUAL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glActiveStencilFaceEXT(GL_FRONT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);

	glActiveStencilFaceEXT(GL_BACK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);

	//The light pass.
    glEnable(GL_LIGHTING);
    RenderScene();
    glPopAttrib();

	//Draw light pos.
	glDisable(GL_LIGHTING);
	glBegin(GL_POINTS);
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(g_pLightPos);
	glEnd();
	glEnable(GL_LIGHTING);
	
	if(g_bShowShadowVolume)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUseProgramObjectARB(m_glslContext);
		glUniform3fARB(m_glslLightPos, g_pLightPos[0], g_pLightPos[1], g_pLightPos[2]);
		RenderModel(g_pBoxVertices, g_pBoxNormals, (sizeof(g_pBoxVertices) / 4) / 3);
		RenderModel(g_pQuadrilateralsVertices, g_pQuadrilateralsNormals, (sizeof(g_pQuadrilateralsVertices) / 4) / 3);
		glUseProgramObjectARB((GLhandleARB)NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	SDL_GL_SwapBuffers();
}

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	printf("SDL OpenGL GPU Z-Fail Shadows\n");
	printf("2006 Andreas T Jonsson\n\n");
	
	printf("Initializing SDL...\n");
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
	printf("Initializing Video...\n");
	SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);
	SDL_WM_SetCaption("SDL OpenGL GPU ZFail", "SDL OpenGL GPU ZFail");
	SDL_ShowCursor(true);
	
	printf("Program Initialize...\n");
    if(!Initialize())
		exit(-1);

    bool bRun = true;
    SDL_Event Event;
	
	printf("Running Program...\n");
	while(bRun)
	{
		SDL_PollEvent(&Event);
		switch(Event.type)
		{
			case SDL_QUIT:
			{
				bRun = false;
			}
			break;

			case SDL_MOUSEBUTTONDOWN:
			{
				if(Event.button.button == SDL_BUTTON_LEFT)
				{
					g_iLastMouseX 	= Event.motion.x;
					g_iLastMouseY 	= Event.motion.y;
					g_bLeftMouse 	= true;
				}
			}
			break;

			case SDL_MOUSEBUTTONUP:
			{
				if(Event.button.button == SDL_BUTTON_LEFT)
					g_bLeftMouse = false;
			}
			break;

			case SDL_MOUSEMOTION:
			{

			}
			break;

			case SDL_KEYDOWN:
			{
				if(Event.key.keysym.sym == SDLK_ESCAPE)
					bRun = false;
				else if(Event.key.keysym.sym == SDLK_F1)
					g_bShowShadowVolume = true;
				else if(Event.key.keysym.sym == SDLK_F2)
					g_bShowShadowVolume = false;
				else if(Event.key.keysym.sym == SDLK_KP_PLUS)
					g_fZoom += 0.01f;
				else if(Event.key.keysym.sym == SDLK_KP_MINUS)
					g_fZoom -= 0.01f;
				else if(Event.key.keysym.sym == SDLK_UP)
					g_pLightPos[2] -= 0.01f;
				else if(Event.key.keysym.sym == SDLK_DOWN)
					g_pLightPos[2] += 0.01f;
				else if(Event.key.keysym.sym == SDLK_LEFT)
					g_pLightPos[0] -= 0.01f;
				else if(Event.key.keysym.sym == SDLK_RIGHT)
					g_pLightPos[0] += 0.01f;
			}
			break;
            
			default:
				break;
		}

		Render();
	}
	
	printf("Shutdown...\n");
	return 0;
}
