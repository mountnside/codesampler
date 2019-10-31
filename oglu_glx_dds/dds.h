//-----------------------------------------------------------------------------
//           Name: dds.h
//         Author: Andreas T Jonsson
//  Last Modified: 06/09/06
//    Description:  This sample demonstrates how to load DDS textures under Linux,
//                  without using any DX headers. It is based on Kevin Harris
//                  'ogl_glx_sample' and the 'ogl_dds_texture_loader' so check them
//                  out first.
//
//   Control Keys: 
//-----------------------------------------------------------------------------

#ifndef _DDS_H_
#define _DDS_H_

#ifndef MAKEFOURCC
	#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | \
		((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24))
#endif

//FOURCC codes for DX compressed-texture pixel formats.
#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT2 MAKEFOURCC('D', 'X', 'T', '2')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT4 MAKEFOURCC('D', 'X', 'T', '4')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')

#define CR_DDS_PIXEL_DATA_OFFSET 128
#define CR_FOURCC_OFFSET 84
#define CR_MIPMAP_COUNT_OFFSET 28
#define CR_LINEAR_SIZE_OFFSET 20
#define CR_DDS_WIDTH_OFFSET 16
#define CR_DDS_HEIGHT_OFFSET 12

struct DDSData
{
	int				m_iWidth;
	int				m_iHeight;
	int				m_iComponents;
	GLenum			m_Format;
	int				m_iNumMipMaps;
	unsigned char	*m_pPixels;
};

class DDS
{
	public:
		bool Create		(const char *pFileName, unsigned int uiMin = GL_LINEAR_MIPMAP_LINEAR, unsigned int uiMag = GL_LINEAR);
		void Destroy	();

		int				m_iWidth;
		int				m_iHeight;
		unsigned int	m_uiId;
		GLenum			m_Format;

	private:
		DDSData *ReadCompressedTexture(const char *pFileName);
};

#endif //_DDS_H_
