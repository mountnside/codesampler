//-----------------------------------------------------------------------------
// The Frame Buffer Object Class.
//-----------------------------------------------------------------------------

#ifndef _FBO_H_
#define _FBO_H_

//Super hack stuff! ;)
#define GL_DEPTH_STENCIL_EXT		0x84F9
#define GL_UNSIGNED_INT_24_8_EXT	0x84FA
#define GL_DEPTH24_STENCIL8_EXT		0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT 0x88F1

class Fbo
{
	public:
		bool			Initialize		(int iWidth, int iHeight);
		unsigned int	GetColorTexture	();
		void			Attach			();
		void			Detach			();
		void			Destroy			();

	private:
		unsigned int m_uiFrameBuffer;
		unsigned int m_uiDepthAndStencilBuffer;

		unsigned int m_uiColorTextureID;
		unsigned int m_uiDepthTextureID;

};

#endif