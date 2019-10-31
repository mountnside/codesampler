#include "main.h"

//-----------------------------------------------------------------------------
// Name: Initialize(int iWidth, int iHeight)
// Desc: Initialize the fbo.
//-----------------------------------------------------------------------------

bool Fbo::Initialize(int iWidth, int iHeight)
{
	glGenFramebuffersEXT(1, &m_uiFrameBuffer);
	glGenRenderbuffersEXT(1, &m_uiDepthAndStencilBuffer);

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_uiDepthAndStencilBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, iWidth, iHeight);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, iWidth, iHeight);

	//Create a color texture.
	glGenTextures(1, &m_uiColorTextureID);
	glBindTexture(GL_TEXTURE_2D, m_uiColorTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Create a depth texture.
	glGenTextures(1, &m_uiDepthTextureID);
	glBindTexture(GL_TEXTURE_2D, m_uiDepthTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, iWidth, iHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
}

//-----------------------------------------------------------------------------
// Name: Attach()
// Desc: Attaches the fbo.
//-----------------------------------------------------------------------------

void Fbo::Attach()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_uiFrameBuffer);

	//Bind the textures.
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_uiColorTextureID, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_uiDepthTextureID, 0);

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_uiDepthAndStencilBuffer);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_uiDepthAndStencilBuffer);
}

//-----------------------------------------------------------------------------
// Name: Detach()
// Desc: Detaches the fbo.
//-----------------------------------------------------------------------------

void Fbo::Detach()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc: Destroys the fbo.
//-----------------------------------------------------------------------------

void Fbo::Destroy()
{
	glDeleteTextures(1, &m_uiColorTextureID);
	glDeleteTextures(1, &m_uiDepthTextureID);
	glDeleteFramebuffersEXT(1, &m_uiFrameBuffer);
	glDeleteRenderbuffersEXT(1, &m_uiDepthAndStencilBuffer);
}

//-----------------------------------------------------------------------------
// Name: GetColorTexture()
// Desc: Returns the color texture.
//-----------------------------------------------------------------------------

unsigned int Fbo::GetColorTexture()
{
	return m_uiColorTextureID;
}
