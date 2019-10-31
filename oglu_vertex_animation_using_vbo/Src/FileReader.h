//-----------------------------------------------------------------------------
//         Author: Tomas Dirvanauskas (aka Tactic)
//    Description: Loads file to RAM, and reads it from there, thus doing it much
//				   faster than simple method
//-----------------------------------------------------------------------------

#ifndef __FILEREADER__
#define __FILEREADER__

#include "Types.h"
#include "Vector3D.h"
#include "Vector2D.h"
#include <stdio.h>			//For file
#include <string.h>			//For memcpy

//Simple class for reading data from memory, not from your C: disk (it's much faster)

class CFileReader
{
	unsigned	char		*m_pcBuffer;

	unsigned	 m_nCurPtr;

	unsigned	 m_nSize;
public:
	CFileReader();
	~CFileReader();
	//Loads file into the memory
	bool		OpenFile(const char *strName);
	void		CloseFile();

	const	unsigned&	GetCurPtr()const{return m_nCurPtr;}
	//Returns size in bytes
	const	unsigned&	GetSize()  const{return m_nSize;}

	const	unsigned char*GetBuffer()const{return m_pcBuffer;}


	inline void ReadBuffer(void* pcBuffer,unsigned nCount)
	{
		memcpy(pcBuffer,&m_pcBuffer[m_nCurPtr],nCount);
		m_nCurPtr+=nCount;
	}

	inline void ReadString(char* strResult,unsigned nCount)
	{
		unsigned nCurrent = 0;
		while(m_pcBuffer[m_nCurPtr]!=13 && m_pcBuffer[m_nCurPtr]!=0 && nCurrent+1<nCount)
		{
			strResult[nCurrent] = m_pcBuffer[m_nCurPtr];
			nCurrent++;
			m_nCurPtr++;
		}

		m_nCurPtr++;
		strResult[nCurrent] = '\0';
	}

	inline int8  ReadInt8()
	{
		return m_pcBuffer[m_nCurPtr++];
	}
	inline uint8  ReadUint8()
	{
		return uint8(m_pcBuffer[m_nCurPtr++]);
	}
	inline int32 ReadInt32()
	{
		int32 n=*(int32*)&m_pcBuffer[m_nCurPtr];
		m_nCurPtr+=4;
		return n;
	}
	inline int32 ReadUint32()
	{
		uint32 n=*(uint32*)&m_pcBuffer[m_nCurPtr];
		m_nCurPtr+=4;
		return n;
	}
	inline float CFileReader::ReadFloat32()
	{
		float f=*(float*)&m_pcBuffer[m_nCurPtr];
		m_nCurPtr+=4;
		return f;
	}

	inline Vector3D CFileReader::ReadVector3D()
	{
		Vector3D vResult;
		vResult.x = ReadFloat32();
		vResult.y = ReadFloat32();
		vResult.z = ReadFloat32();
		return vResult;
	}

	inline Vector2D CFileReader::ReadVector2D()
	{
		Vector2D vResult;
		vResult.x = ReadFloat32();
		vResult.y = ReadFloat32();
		return vResult;
	}

};


#endif