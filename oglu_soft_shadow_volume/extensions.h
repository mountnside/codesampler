//-----------------------------------------------------------------------------
// The extension header.
//-----------------------------------------------------------------------------

#ifndef _EXTENSIONS_H_
#define _EXTENSIONS_H_

//Framebuffer.
extern PFNGLBINDRENDERBUFFEREXTPROC			glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC		glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC			glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC		glRenderbufferStorageEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC			glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC		glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC			glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC		glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRenderbufferEXT;

//GPU programs.
extern PFNGLCREATEPROGRAMOBJECTARBPROC	glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC	glCreateShaderObjectARB;
extern PFNGLLINKPROGRAMARBPROC			glLinkProgramARB;
extern PFNGLCOMPILESHADERARBPROC		glCompileShaderARB;
extern PFNGLDELETEOBJECTARBPROC			glDeleteObjectARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgramObjectARB;
extern PFNGLSHADERSOURCEARBPROC			glShaderSourceARB;
extern PFNGLATTACHOBJECTARBPROC			glAttachObjectARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameterivARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC	glGetUniformLocationARB;
extern PFNGLUNIFORM1FARBPROC			glUniform1fARB;
extern PFNGLUNIFORM1IARBPROC			glUniform1iARB;

extern bool InitializeExtensions();

#endif