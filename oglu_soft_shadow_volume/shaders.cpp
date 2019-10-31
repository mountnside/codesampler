#include "main.h"

//-----------------------------------------------------------------------------
// Name: Initialize(const char *pBlurVS, const char *pBlurPS)
// Desc: Initialize the shaders.
//-----------------------------------------------------------------------------

bool Shaders::Initialize(const char *pBlurVS, const char *pBlurPS)
{
	int iResult;

	//Create program object.
	m_glslContext = glCreateProgramObjectARB();

	//Load vertex shader.
	char *pShaderCode = LoadShader(pBlurVS);
	if(!pShaderCode)
		return false;

	m_glslVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glShaderSourceARB(m_glslVertexShader, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(m_glslVertexShader);
	glGetObjectParameterivARB(m_glslVertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(m_glslContext, m_glslVertexShader);

	DestroyShader(pShaderCode);

	//Load pixel shader.
	pShaderCode = LoadShader(pBlurPS);
	if(!pShaderCode)
		return false;

	m_glslPixelShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(m_glslPixelShader, 1, (const char**)&pShaderCode, NULL);
	glCompileShaderARB(m_glslPixelShader);
	glGetObjectParameterivARB(m_glslPixelShader, GL_OBJECT_COMPILE_STATUS_ARB, &iResult);
	glAttachObjectARB(m_glslContext, m_glslPixelShader);

	DestroyShader(pShaderCode);

	//Link all the shaders.
	glLinkProgramARB(m_glslContext);
	glGetObjectParameterivARB(m_glslContext, GL_OBJECT_LINK_STATUS_ARB, &iResult);
	if(!iResult)
		return false;

	//Bind the shaders variables to our variables.
	m_glslSampleDist			= glGetUniformLocationARB(m_glslContext, "uSampleDist");
	m_glslFrameBufferTexture	= glGetUniformLocationARB(m_glslContext, "uFrameBufferTexture");

	return true;
}

//-----------------------------------------------------------------------------
// Name: LoadShader(const char *pFileName)
// Desc:
//-----------------------------------------------------------------------------

char *Shaders::LoadShader(const char *pFileName)
{
	FILE *pFile = fopen(pFileName, "r");
	if(!pFile)
		return NULL;

	struct _stat FileStats;
	if(_stat(pFileName, &FileStats) != 0)
	{
		fclose(pFile);
		return NULL;
	}

	char *pData = new char[FileStats.st_size];

	int iBytes = fread(pData, 1, FileStats.st_size, pFile);
	pData[iBytes] = '\0';
	fclose(pFile);

	return pData;
}

//-----------------------------------------------------------------------------
// Name: DestroyShader(char *pShader)
// Desc:
//-----------------------------------------------------------------------------

void Shaders::DestroyShader(char *pShader)
{
	delete[] pShader;
	pShader = NULL;
}

//-----------------------------------------------------------------------------
// Name: Blur(float fSampleDist, unsigned int uiFrameBufferTexture)
// Desc:
//-----------------------------------------------------------------------------

void Shaders::Blur(float fSampleDist, unsigned int uiFrameBufferTexture)
{
	glUseProgramObjectARB(m_glslContext);

	glUniform1fARB(m_glslSampleDist, fSampleDist);
	glUniform1iARB(m_glslFrameBufferTexture, 0);

	glBindTexture(GL_TEXTURE_2D, uiFrameBufferTexture);
}

//-----------------------------------------------------------------------------
// Name: Disable()
// Desc:
//-----------------------------------------------------------------------------

void Shaders::Disable()
{
	glUseProgramObjectARB(NULL);
}

//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc:
//-----------------------------------------------------------------------------

void Shaders::Destroy()
{
	glDeleteObjectARB(m_glslPixelShader);
	glDeleteObjectARB(m_glslContext);
}
