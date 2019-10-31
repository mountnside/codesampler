//-----------------------------------------------------------------------------
//           Name: sdlu_ogl_gpu_soft_zfail.cpp
//         Author: Andreas T Jonsson
//  Last Modified: 2007
//    Description: This sample demonstrates how to render soft z-fail shadows
//                 on the GPU, that doesn't glow! :)
//-----------------------------------------------------------------------------

/*
	Before you start with this tutorial you should have good knowledge of GLSL,
	the z-fail algorythem and GPU extrusion. You can find tutorials on those 
    subjects at http://www.codesampler.com.

	To achive the desired resault, this demo uses a technic called smartblur.
	It works by comparing the depth of the current fragment with it's neighbors.
	If the delta depth is abow a user defined threshold the sample is rejected.
*/

#include <iostream>

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "geometry.h"

//-----------------------------------------------------------------------------

#define RENDER_BUFFER_SIZE 512

//-----------------------------------------------------------------------------

//Two side stencil buffer extension.
PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT = NULL;

//GPU programs.
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
PFNGLUNIFORM3FARBPROC				glUniform3fARB				= NULL;
PFNGLUNIFORM1FARBPROC				glUniform1fARB				= NULL;
PFNGLUNIFORM1IARBPROC				glUniform1iARB				= NULL;

//-----------------------------------------------------------------------------

struct V3D
{
	float x, y, z;
};

//-----------------------------------------------------------------------------

bool g_bLeftMouse 			= false;
bool g_bShowShadowVolume 	= false;
bool g_bTwoPassBlur 		= true;

int g_iLastMouseX = 0;
int g_iLastMouseY = 0;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

float g_fZoom = -6.0f;
float g_pLightPos[] = {1.2f, 1.5f, 1.6f};

GLhandleARB g_glslExtrusionVertexProgram;
GLhandleARB g_glslExtrusionFragmentProgram;
GLhandleARB g_glslExtrusionContext;

GLhandleARB g_glslSoftShadowVertexProgram;
GLhandleARB g_glslSoftShadowFragmentProgram;
GLhandleARB g_glslSoftShadowContext;

unsigned int g_glslLightPos;
unsigned int g_glslDepthTexture;
unsigned int g_glslColorTexture;
unsigned int g_glslOffset;

unsigned int g_uiDepthBuffer;
unsigned int g_uiColorBuffer;

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

unsigned int CreateTexture()
{
	unsigned char *pData = new unsigned char[RENDER_BUFFER_SIZE * RENDER_BUFFER_SIZE * 4];
	memset(pData, 0, RENDER_BUFFER_SIZE * RENDER_BUFFER_SIZE * 4);

	unsigned int uiTexture;
	glGenTextures(1, &uiTexture);
	glBindTexture(GL_TEXTURE_2D, uiTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, RENDER_BUFFER_SIZE, RENDER_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	delete[] pData;
	return uiTexture;
}

//-----------------------------------------------------------------------------

bool LoadExtrusionShaders(const char *pVertexProgram, const char *pFragmentProgram)
{
	int iResult;
	char pBuffer[4096];

	//Create program object.
	g_glslExtrusionContext = glCreateProgramObjectARB();

	//Load vertex program.
	char *pShaderCode = LoadShader(pVertexProgram);
	if(!pShaderCode)
	{
		printf("Can't load %s\n", pVertexProgram);
		return false;
	}
	
	g_glslExtrusionVertexProgram = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glShaderSourceARB(g_glslExtrusionVertexProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(g_glslExtrusionVertexProgram);
	glGetObjectParameterivARB(g_glslExtrusionVertexProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(g_glslExtrusionContext, g_glslExtrusionVertexProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(g_glslExtrusionVertexProgram, sizeof(pBuffer), NULL, pBuffer);
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

	g_glslExtrusionFragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(g_glslExtrusionFragmentProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(g_glslExtrusionFragmentProgram);
	glGetObjectParameterivARB(g_glslExtrusionFragmentProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(g_glslExtrusionContext, g_glslExtrusionFragmentProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(g_glslExtrusionFragmentProgram, sizeof(pBuffer), NULL, pBuffer);
		printf("Fragment Program Error:\n");
		printf("%s\n", pBuffer);
		return false;
	}

	//Link all the shaders.
	glLinkProgramARB(g_glslExtrusionContext);
	glGetObjectParameterivARB(g_glslExtrusionContext, GL_OBJECT_LINK_STATUS_ARB, &iResult);
	if(!iResult)
	{
		glGetInfoLogARB(g_glslExtrusionContext, sizeof(pBuffer), NULL, pBuffer);
		printf("%s\n", pBuffer);
		return false;
	}
	
	//Get the uniform.
	g_glslLightPos = glGetUniformLocationARB(g_glslExtrusionContext, "uLightPos");
	
	return true;
}

//-----------------------------------------------------------------------------

bool LoadSoftShadowShaders(const char *pVertexProgram, const char *pFragmentProgram)
{
	int iResult;
	char pBuffer[4096];

	//Create program object.
	g_glslSoftShadowContext = glCreateProgramObjectARB();

	//Load vertex program.
	char *pShaderCode = LoadShader(pVertexProgram);
	if(!pShaderCode)
	{
		printf("Can't load %s\n", pVertexProgram);
		return false;
	}
	
	g_glslSoftShadowVertexProgram = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glShaderSourceARB(g_glslSoftShadowVertexProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(g_glslSoftShadowVertexProgram);
	glGetObjectParameterivARB(g_glslSoftShadowVertexProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(g_glslSoftShadowContext, g_glslSoftShadowVertexProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(g_glslSoftShadowVertexProgram, sizeof(pBuffer), NULL, pBuffer);
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

	g_glslSoftShadowFragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(g_glslSoftShadowFragmentProgram, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(g_glslSoftShadowFragmentProgram);
	glGetObjectParameterivARB(g_glslSoftShadowFragmentProgram, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(g_glslSoftShadowContext, g_glslSoftShadowFragmentProgram);

	delete[] pShaderCode;
	
	if(!iResult)
	{
		glGetInfoLogARB(g_glslSoftShadowFragmentProgram, sizeof(pBuffer), NULL, pBuffer);
		printf("Fragment Program Error:\n");
		printf("%s\n", pBuffer);
		return false;
	}

	//Link all the shaders.
	glLinkProgramARB(g_glslSoftShadowContext);
	glGetObjectParameterivARB(g_glslSoftShadowContext, GL_OBJECT_LINK_STATUS_ARB, &iResult);
	if(!iResult)
	{
		glGetInfoLogARB(g_glslSoftShadowContext, sizeof(pBuffer), NULL, pBuffer);
		printf("%s\n", pBuffer);
		return false;
	}
	
	//Get the uniform.
	g_glslColorTexture = glGetUniformLocationARB(g_glslSoftShadowContext, "uColorTexture");
	g_glslDepthTexture = glGetUniformLocationARB(g_glslSoftShadowContext, "uDepthTexture");
	
	g_glslOffset = glGetUniformLocationARB(g_glslSoftShadowContext, "uOffset");
	
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
	glUniform1iARB				= (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
	glUniform1fARB				= (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");

	glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)SDL_GL_GetProcAddress("glActiveStencilFaceEXT");

	//glActiveTexture = (PFNglActiveTexturePROC)SDL_GL_GetProcAddress("glActiveTexture");

*(void**) & glActiveTexture = SDL_GL_GetProcAddress( "glActiveTexture" ) ;

	//Check if extensions fails.
	if( !(glCreateProgramObjectARB && glCreateShaderObjectARB && 
          glCompileShaderARB && glLinkProgramARB && glUniform1iARB &&
		  glGetInfoLogARB && glUseProgramObjectARB && glShaderSourceARB && 
          glAttachObjectARB && glActiveTexture && glGetObjectParameterivARB && 
          glGetUniformLocationARB && glUniform3fARB && glUniform1fARB && 
          glActiveStencilFaceEXT) )
	{
        printf("Missing one or more OpenGL Extensions required for GLSL shaders!\n");
		exit(-1);
	}
	
	if(!LoadExtrusionShaders("./extrude.vp", "./extrude.fp"))
		exit(-1);
	
	if(!LoadSoftShadowShaders("./softshadows.vp", "./softshadows.fp"))
		exit(-1);
	
	//Create our render targets.
	g_uiColorBuffer = CreateTexture();
	g_uiDepthBuffer = CreateTexture();
	
	//Setup OpenGL.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 800.0f / 600.0f, 1.0f, -1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPointSize(5.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

void RenderQuad(int iTexture)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(false);
	//glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	if(iTexture > -1)
		glBindTexture(GL_TEXTURE_2D, (unsigned int)iTexture);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}
//-----------------------------------------------------------------------------

void RenderWhiteQuad()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(false);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

//-----------------------------------------------------------------------------

void RenderScene()
{
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);
	
	//Render floor.
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		
		glVertex3f(-2.0f, -1.5f, -2.0f);
		glVertex3f(-2.0f, -1.5f, 2.0f);
		glVertex3f(2.0f, -1.5f, 2.0f);
		glVertex3f(2.0f, -1.5f, -2.0f);
	glEnd();
	
	//Somthing to shadow.
	glCullFace(GL_FRONT);
	glPushMatrix();
	glTranslatef(-1.2f, -0.75f, 0.0f);
	glRotatef(55.0f, 0.0f, 1.0f, 0.0f);
	renderSolidTeapot(1.0f);
	glPopMatrix();
	
	//The shadow caster.
	glCullFace(GL_BACK);
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
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	//Fill the depth buffer. And save it!
	glDisable(GL_LIGHTING);
	RenderScene();
	glBindTexture(GL_TEXTURE_2D, g_uiDepthBuffer);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, 800, 600, 0);
	
	//Prevents z-fighting.
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f, 100.0f);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT); //Use the two side stencil buffer.
	
	///Incrementing the stencil buffer.
	glActiveStencilFaceEXT(GL_FRONT);
	glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0);

	//Decrement the stencil buffer.
	glActiveStencilFaceEXT(GL_BACK);
	glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0);

	//Render shadow-volume with our shader.
	glUseProgramObjectARB(g_glslExtrusionContext);
	glUniform3fARB(g_glslLightPos, g_pLightPos[0], g_pLightPos[1], g_pLightPos[2]);
	RenderModel(g_pBoxVertices, g_pBoxNormals, (sizeof(g_pBoxVertices) / 4) / 3);
	RenderModel(g_pQuadrilateralsVertices, g_pQuadrilateralsNormals, (sizeof(g_pQuadrilateralsVertices) / 4) / 3);
	glUseProgramObjectARB((GLhandleARB)NULL);
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glActiveStencilFaceEXT(GL_FRONT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);

	glActiveStencilFaceEXT(GL_BACK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);
	
	RenderWhiteQuad();
	
	//Copy the color buffer.
	glBindTexture(GL_TEXTURE_2D, g_uiColorBuffer);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 800, 600, 0);
	
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	glPopAttrib();
	
	//Do soft shadows.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glUseProgramObjectARB(g_glslSoftShadowContext);
	glUniform1iARB(g_glslColorTexture, 0);
	glUniform1iARB(g_glslDepthTexture, 1);
	
	glUniform1fARB(g_glslOffset, 0.008f);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_uiColorBuffer);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_uiDepthBuffer);

	RenderQuad(-1);
		
	//Copy the color buffer.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_uiColorBuffer);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 800, 600, 0);
	
	if(g_bTwoPassBlur)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_uiColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_uiDepthBuffer);
		
		glUniform1fARB(g_glslOffset, (GLfloat)0.01);
		RenderQuad(-1);
		
		//Copy the color buffer.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_uiColorBuffer);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 800, 600, 0);		
	}
	
	glUseProgramObjectARB((GLhandleARB)NULL);
	
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//The light pass.
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
    RenderScene();
	glDisable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	
	RenderQuad(g_uiColorBuffer);
	glPopAttrib();
		
	//Draw light pos.
	glDisable(GL_LIGHTING);
	glBegin(GL_POINTS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(g_pLightPos);
	glEnd();
	glEnable(GL_LIGHTING);
	
	if(g_bShowShadowVolume)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUseProgramObjectARB(g_glslExtrusionContext);
		glUniform3fARB(g_glslLightPos, g_pLightPos[0], g_pLightPos[1], g_pLightPos[2]);
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
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	printf("Initializing Video...\n");
	SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);
	SDL_WM_SetCaption("SDL OpenGL Soft GPU ZFail", "SDL OpenGL GPU ZFail");
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
				else if(Event.key.keysym.sym == SDLK_F3)
					g_bTwoPassBlur = true;
				else if(Event.key.keysym.sym == SDLK_F4)
					g_bTwoPassBlur = false;
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
