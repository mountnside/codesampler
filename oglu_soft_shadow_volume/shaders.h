//-----------------------------------------------------------------------------
// The Shader Class.
//-----------------------------------------------------------------------------

#ifndef _SHADERS_H_
#define _SHADERS_H_

class Shaders
{
	public:
		bool Initialize	(const char *pBlurVS, const char *pBlurPS);
		void Blur		(float fSampleDist, unsigned int uiFrameBufferTexture);
		void Disable	();
		void Destroy	();

	private:
		char *LoadShader	(const char *pFileName);
		void DestroyShader	(char *pShader);

		GLhandleARB m_glslContext;
		GLhandleARB m_glslVertexShader;
		GLhandleARB m_glslPixelShader;

		unsigned int m_glslSampleDist;
		unsigned int m_glslFrameBufferTexture;
};

#endif
