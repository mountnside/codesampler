#include "main.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

//Framebuffer.
PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT			= NULL;
PFNGLDELETERENDERBUFFERSEXTPROC		glDeleteRenderbuffersEXT		= NULL;
PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT			= NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC		glRenderbufferStorageEXT		= NULL;
PFNGLBINDFRAMEBUFFEREXTPROC			glBindFramebufferEXT			= NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC		glDeleteFramebuffersEXT			= NULL;
PFNGLGENFRAMEBUFFERSEXTPROC			glGenFramebuffersEXT			= NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	glCheckFramebufferStatusEXT		= NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC	glFramebufferTexture2DEXT		= NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRenderbufferEXT	= NULL;

//GPU programs.
PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgramObjectARB	= NULL;
PFNGLCREATESHADEROBJECTARBPROC		glCreateShaderObjectARB		= NULL;
PFNGLLINKPROGRAMARBPROC				glLinkProgramARB			= NULL;
PFNGLCOMPILESHADERARBPROC			glCompileShaderARB			= NULL;
PFNGLDELETEOBJECTARBPROC			glDeleteObjectARB			= NULL;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgramObjectARB		= NULL;
PFNGLSHADERSOURCEARBPROC			glShaderSourceARB			= NULL;
PFNGLATTACHOBJECTARBPROC			glAttachObjectARB			= NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameterivARB	= NULL;
PFNGLGETUNIFORMLOCATIONARBPROC		glGetUniformLocationARB		= NULL;
PFNGLUNIFORM1FARBPROC				glUniform1fARB				= NULL;
PFNGLUNIFORM1IARBPROC				glUniform1iARB				= NULL;

//-----------------------------------------------------------------------------
// Name: InitializeExtensions()
// Desc: This initializes all extensions.
//-----------------------------------------------------------------------------

bool InitializeExtensions()
{
	//Framebuffer.
	glBindRenderbufferEXT			= (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT		= (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
	glGenRenderbuffersEXT			= (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glRenderbufferStorageEXT		= (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glBindFramebufferEXT			= (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glDeleteFramebuffersEXT			= (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	glGenFramebuffersEXT			= (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT		= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture2DEXT		= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glFramebufferRenderbufferEXT	= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");

	//GPU programs.
	glCreateProgramObjectARB	= (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glCreateShaderObjectARB		= (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glCompileShaderARB			= (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glLinkProgramARB			= (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glDeleteObjectARB			= (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
	glUseProgramObjectARB		= (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glShaderSourceARB			= (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glGetObjectParameterivARB	= (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glGetUniformLocationARB		= (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
	glUniform1fARB				= (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
	glUniform1iARB				= (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");

	if(!glBindRenderbufferEXT || !glDeleteRenderbuffersEXT || !glGenRenderbuffersEXT || !glRenderbufferStorageEXT || !glBindFramebufferEXT ||
		!glDeleteFramebuffersEXT || !glGenFramebuffersEXT || !glCheckFramebufferStatusEXT || !glFramebufferTexture2DEXT || !glFramebufferRenderbufferEXT) return false;

	if(!glCreateProgramObjectARB || !glCreateShaderObjectARB || !glCompileShaderARB || !glLinkProgramARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
		!glShaderSourceARB || !glAttachObjectARB || !glGetObjectParameterivARB || !glGetUniformLocationARB || !glUniform1fARB || !glUniform1iARB) return false;

	return true;
}
