//-----------------------------------------------------------------------------
//           Name: dds.cpp
//         Author: Andreas T Jonsson
//  Last Modified: 06/09/06
//    Description:  This sample demonstrates how to load DDS textures under Linux,
//                  without using any DX headers. It is based on Kevin Harris
//                  'ogl_glx_sample' and the 'ogl_dds_texture_loader' so check them
//                  out first.
//
//   Control Keys: 
//-----------------------------------------------------------------------------

#include "main.h"

//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc:
//-----------------------------------------------------------------------------
void DDS::Destroy()
{
	glDeleteTextures(1, &m_uiId);
}

//-----------------------------------------------------------------------------
// Name: Create(const char *pFileName, unsigned int uiMin, unsigned int uiMag)
// Desc:
//-----------------------------------------------------------------------------
bool DDS::Create(const char *pFileName, unsigned int uiMin, unsigned int uiMag)
{
	DDSData *pDDSImageData = ReadCompressedTexture(pFileName);

	if(pDDSImageData)
	{
		int iSize;
		int iBlockSize;
		int iOffset		= 0;
		int iHeight		= pDDSImageData->m_iHeight;
		int iWidth		= pDDSImageData->m_iWidth;
		int iNumMipMaps = pDDSImageData->m_iNumMipMaps;

		iBlockSize = (pDDSImageData->m_Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

		glGenTextures(1, &m_uiId);
		glBindTexture(GL_TEXTURE_2D, m_uiId);

		if(iNumMipMaps < 2)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			iNumMipMaps = 1;
		}
		else
		{
			//Set texture filtering.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiMin);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiMag);
		}

		for(int i = 0; i < iNumMipMaps; i++)
		{
			iSize = ((iWidth + 3) / 4) * ((iHeight + 3) / 4) * iBlockSize;
			glCompressedTexImage2DARB(GL_TEXTURE_2D, i, pDDSImageData->m_Format, iWidth, iHeight, 0, iSize, pDDSImageData->m_pPixels + iOffset);
			iOffset += iSize;

			//Scale next level.
			iWidth  /= 2;
			iHeight /= 2;
		}

		m_Format	= pDDSImageData->m_Format;
		m_iWidth	= pDDSImageData->m_iWidth;
		m_iHeight	= pDDSImageData->m_iHeight;

		if(pDDSImageData->m_pPixels)
			delete[] pDDSImageData->m_pPixels;

		delete pDDSImageData;
	}
	else
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Name: ReadCompressedTexture(const char *pFileName)
// Desc:
//-----------------------------------------------------------------------------
DDSData *DDS::ReadCompressedTexture(const char *pFileName)
{
	FILE *pFile;
	if(!(pFile = fopen(pFileName, "rb")))
		return NULL;

	char pFileCode[4];
	fread(pFileCode, 1, 4, pFile);
	if(strncmp(pFileCode, "DDS ", 4) != 0)
		return NULL;

	//Get the descriptor.
	unsigned int uiFourCC;
	fseek(pFile, CR_FOURCC_OFFSET, SEEK_SET);
	fread(&uiFourCC, sizeof(unsigned int), 1, pFile);

	unsigned int uiLinearSize;
	fseek(pFile, CR_LINEAR_SIZE_OFFSET, SEEK_SET);
	fread(&uiLinearSize, sizeof(unsigned int), 1, pFile);

	unsigned int uiMipMapCount;
	fseek(pFile, CR_MIPMAP_COUNT_OFFSET, SEEK_SET);
	fread(&uiMipMapCount, sizeof(unsigned int), 1, pFile);

	unsigned int uiWidth;
	fseek(pFile, CR_DDS_WIDTH_OFFSET, SEEK_SET);
	fread(&uiWidth, sizeof(unsigned int), 1, pFile);

	unsigned int uiHeight;
	fseek(pFile, CR_DDS_HEIGHT_OFFSET, SEEK_SET);
	fread(&uiHeight, sizeof(unsigned int), 1, pFile);

	int iFactor;
	int iBufferSize;
	
	DDSData *pDDSImageData = new DDSData;
	memset(pDDSImageData, 0, sizeof(DDSData));
	
	switch(uiFourCC)
	{
		case FOURCC_DXT1:
			pDDSImageData->m_Format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			iFactor = 2;
			break;

		case FOURCC_DXT3:
			pDDSImageData->m_Format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			iFactor = 4;
			break;

		case FOURCC_DXT5:
			pDDSImageData->m_Format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			iFactor = 4;
			break;

		default:
			return NULL;
	}
	
	iBufferSize = (uiMipMapCount > 1) ? (uiLinearSize * iFactor) : uiLinearSize;
	pDDSImageData->m_pPixels = new unsigned char[iBufferSize];
	
	pDDSImageData->m_iWidth      = uiWidth;
	pDDSImageData->m_iHeight     = uiHeight;
	pDDSImageData->m_iNumMipMaps = uiMipMapCount;
	pDDSImageData->m_iComponents = (uiFourCC == FOURCC_DXT1) ? 3 : 4;

	fseek(pFile, CR_DDS_PIXEL_DATA_OFFSET, SEEK_SET);
	fread(pDDSImageData->m_pPixels, 1, iBufferSize, pFile);
	fclose(pFile);

	return pDDSImageData;
}
