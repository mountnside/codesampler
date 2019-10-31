#include "FileReader.h"

CFileReader::CFileReader()
{
	m_pcBuffer = NULL;

	m_nCurPtr = 0;
	m_nSize = 0;
}
CFileReader::~CFileReader()
{
	delete []m_pcBuffer;
}

bool CFileReader::OpenFile(const char *strName)
{
	FILE *fp;
	fopen_s(&fp,strName,"rb");
	
	if(fp)
	{
		fseek (fp, 0, SEEK_END);
		m_nSize = ftell (fp);
		fseek (fp, 0, SEEK_SET);

		m_pcBuffer = new unsigned char[m_nSize];

		fread(m_pcBuffer,sizeof(unsigned char)*m_nSize,1,fp);

		fclose(fp);
		m_nCurPtr = 0;
		return true;
	}
	else return false;
}
void CFileReader::CloseFile()
{
	delete []m_pcBuffer;m_pcBuffer=NULL;
	m_nSize = m_nCurPtr = 0;
}

