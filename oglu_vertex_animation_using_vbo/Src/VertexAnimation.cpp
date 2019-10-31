#include "VertexAnimation.h"
#include "FileReader.h"
#include "OpenGLExtensions.h"

#include <stdarg.h>
#include <assert.h>

//Assert with message info
void AssertMsg(bool condition,const char *fmt, ...) 
{
	if(fmt==NULL)return;

	char strings[0x1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(strings, fmt, ap);
    va_end(ap);

	assert(condition && strings);
}
//Fatal Error
void FatalError(const char *fmt, ...) 
{
	if(fmt==NULL)return;

	char strings[0x1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(strings, fmt, ap);
    va_end(ap);

	assert(false && strings);
}



CVertexAnimation::CMeshNode::CMeshNode()
:m_pvAnimTable(NULL)
,m_nAnimCnt(0)
{
	m_nVBOVertices	= 0;
	m_nVBONormals	= 0;
	m_nVBOIndices	= 0;
}
CVertexAnimation::CMeshNode::~CMeshNode()
{
	if(m_pvAnimTable)
	{
		for(uint32 i=0;i<GetVertexCnt();i++)
		{
			delete m_pvAnimTable[i];
		}
		delete []m_pvAnimTable;
	}
}
void CVertexAnimation::CMeshNode::CalculateVertices(float t)
{
	for(uint32 i=0;i<GetVertexCnt();i++)
	{
		m_pvVertices[i] = LerpVert(i,t);
	}
}
void CVertexAnimation::CMeshNode::CalculateNormals()
{
	for(uint32 i=0;i<GetVertexCnt();i++)m_pvNormals[i] = Vector3D(0.0f,0.0f,0.0f);
	//Our mesh consists of triangles, so GetIndexCnt() can be divided by 3
	//We simply go through all the triangles, calculate normal for it, and add result to the vertex's normal
	Vector3D v[3],vNormal;
	for(uint32 i=0;i<GetIndexCnt();i+=3)
	{
		v[0] = m_pvVertices[GetVertIndex(i+0)];
		v[1] = m_pvVertices[GetVertIndex(i+1)]; 
		v[2] = m_pvVertices[GetVertIndex(i+2)];

		vNormal = Cross(v[1]-v[0],v[2]-v[0]);
		m_pvNormals[GetVertIndex(i+0)]+=vNormal;
		m_pvNormals[GetVertIndex(i+1)]+=vNormal;
		m_pvNormals[GetVertIndex(i+2)]+=vNormal;
	}
	//Normalize our big sum
	for(uint32 i=0;i<GetVertexCnt();i++)m_pvNormals[i].normalize();
}

void CVertexAnimation::CMeshNode::CreateVBOs()
{
	if(GL_1_5_supported)
	{
		AssertMsg(m_nVBOVertices==0 && m_nVBONormals==0 && m_nVBOIndices==0,"VBOs already created");
		glGenBuffersARB( 1, &m_nVBOVertices );	
		glGenBuffersARB( 1, &m_nVBONormals );	
		glGenBuffersARB( 1, &m_nVBOIndices );	

		//Fill it all
		//Vertices - will change every frame so we will use GL_STREAM_DRAW
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, GetVertexCnt()*sizeof(Vector3D), &m_pvVertices[0], GL_STREAM_DRAW );
		//Normals - will change every frame so we will use GL_STREAM_DRAW
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBONormals );			
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, GetVertexCnt()*sizeof(Vector3D), &m_pvNormals[0], GL_STREAM_DRAW );

		//Indices - changes only on loading, so we'll use GL_STATIC_DRAW
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_nVBOIndices );			
		glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, GetIndexCnt()*sizeof(uint32), &m_pnIndices[0], GL_STATIC_DRAW );
	}


}
void CVertexAnimation::CMeshNode::DeleteVBOs()
{
	if(GL_1_5_supported)
	{
		AssertMsg(m_nVBOVertices!=0 && m_nVBONormals!=0 && m_nVBOIndices!=0,"VBOs were not created");
		glDeleteBuffersARB( 1,&m_nVBOVertices );
		glDeleteBuffersARB( 1,&m_nVBONormals );
		glDeleteBuffersARB( 1,&m_nVBOIndices );

		m_nVBOVertices	= 0;
		m_nVBONormals	= 0;
		m_nVBOIndices	= 0;

	}
}
void CVertexAnimation::CMeshNode::UpdateVertices()
{
	if(GL_1_5_supported)
	{
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			
		glBufferSubData( GL_ARRAY_BUFFER_ARB, 0,GetVertexCnt()*sizeof(Vector3D), &m_pvVertices[0] );
	}
}
void CVertexAnimation::CMeshNode::UpdateNormals()
{
	if(GL_1_5_supported)
	{
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBONormals );			
		glBufferSubData( GL_ARRAY_BUFFER_ARB, 0,GetVertexCnt()*sizeof(Vector3D), &m_pvNormals[0] );
	}

}



CVertexAnimation::CVertexAnimation(const char *strFileName)
:m_pcMeshNode(NULL)
,m_fProgress(0.0f)
{
	//Some mambo jambo
	//Loads mesh animation file 

	CFileReader cFile;

	if(cFile.OpenFile(strFileName)==false)
	{
		FatalError("Could not open %s",strFileName);
	}
	uint32 nVersion,j,k,temp;
	char   szBuf[255];
	
	cFile.ReadString(szBuf,255);
	AssertMsg(strcmp(szBuf,"MESHANIM")==0,"This is not an mesh animation file, extension %s",szBuf);
	nVersion = cFile.ReadUint32();
	AssertMsg(nVersion==100,"Incorrect point animation version, theirs %d - ours %d",nVersion,100);
	uint32 nNodeCnt = cFile.ReadUint32();
	AssertMsg(nNodeCnt>0,"Mesh contains zero geometry");
	
	//Load single node(the file may contain more, but this is just a demo...)
	m_pcMeshNode = new CMeshNode();

	//Read initial verts
	m_pcMeshNode->m_pvInitialVerts.resize(cFile.ReadUint32());
	//Prepare cache
	m_pcMeshNode->m_pvNormals.resize(m_pcMeshNode->m_pvInitialVerts.size());
	m_pcMeshNode->m_pvVertices.resize(m_pcMeshNode->m_pvInitialVerts.size());

	//Read initial vertices
	for(j=0;j<uint32(m_pcMeshNode->m_pvInitialVerts.size());j++)
	{
		m_pcMeshNode->m_pvInitialVerts[j] = cFile.ReadVector3D();
	}

	//Read uv's (skip it)
	temp = cFile.ReadInt32();
	for(j=0;j<temp;j++)cFile.ReadVector2D();

	//Read Colors(skip it)
	temp = cFile.ReadInt32();
	for(j=0;j<temp;j++)cFile.ReadVector3D();
	
	//Read indices
	m_pcMeshNode->m_pnIndices.resize(cFile.ReadInt32());
	for(j=0;j<uint32(m_pcMeshNode->m_pnIndices.size());j++)
	{
		m_pcMeshNode->m_pnIndices[j] = cFile.ReadUint32();
		cFile.ReadUint32();						  //Index to uv
		cFile.ReadUint32();						  //Index to color
	}
	//How many frames do we have
	m_pcMeshNode->m_nAnimCnt = cFile.ReadUint32();

	//Animation in file is stored in columns for evey vertex
	//We need them in rows, so first loaded,then distribute them in interpolation table
	//Store animation here (temporary data)
	std::vector<Vector3D>**		pvAnimVerts; //[Frame][Vertex]
	pvAnimVerts = new std::vector<Vector3D>*[m_pcMeshNode->m_nAnimCnt];

	for(k=0;k<m_pcMeshNode->m_nAnimCnt;k++)
	{
		pvAnimVerts[k] = new std::vector<Vector3D>();

		std::vector<Vector3D>& sAnimVerts	   = *(pvAnimVerts[k]);	

		cFile.ReadUint32();					//Read frame

		sAnimVerts.resize(m_pcMeshNode->GetVertexCnt());
		for(j=0;j<(uint32)m_pcMeshNode->GetVertexCnt();j++)
		{
			sAnimVerts[j] = cFile.ReadVector3D();
		}
	}

	//If there's an animation, build animation table
	if(m_pcMeshNode->m_nAnimCnt>0)
	{
		//This helps with delta time
		m_pcMeshNode->m_fDeltaTimeModifier = 30.0f/float(m_pcMeshNode->m_nAnimCnt);

		Vector3D *pvTemp;

		//Create animation tables
		m_pcMeshNode->m_pvAnimTable = new InterpolationTable*[m_pcMeshNode->GetVertexCnt()];
		for(j=0;j<m_pcMeshNode->GetVertexCnt();j++)
		{
			//Gather animation for vertex [j]
			pvTemp = new Vector3D[m_pcMeshNode->m_nAnimCnt];
			for(k=0;k<m_pcMeshNode->m_nAnimCnt;k++)
				pvTemp[k] =  (*pvAnimVerts[k])[j];    
			//Create&build table
			m_pcMeshNode->m_pvAnimTable[j] = new InterpolationTable();
			m_pcMeshNode->m_pvAnimTable[j]->CreateTable(
				m_pcMeshNode->m_nAnimCnt,pvTemp,0.0f,1.0f);

			delete []pvTemp;
		}

		//Delete unused data
		for(k=0;k<m_pcMeshNode->m_nAnimCnt;k++)
		{
			delete pvAnimVerts[k];
		}
		delete [](pvAnimVerts);
	
	}
	else
	{
		m_pcMeshNode->m_fDeltaTimeModifier = 0.0f;
	}
	cFile.CloseFile();
	//Calculate initial data
	m_pcMeshNode->CalculateVertices(0.0f);
	m_pcMeshNode->CalculateNormals();

	m_pcMeshNode->CreateVBOs();
	
}
CVertexAnimation::~CVertexAnimation()
{
	m_pcMeshNode->DeleteVBOs();
	delete m_pcMeshNode;
}


void	CVertexAnimation::Update(float fDeltaTime)
{
	m_fProgress = fmodf(m_fProgress+fDeltaTime*m_pcMeshNode->GetDeltaTimeMod()*0.25f,1.0f);

	m_pcMeshNode->CalculateVertices(m_fProgress);
	m_pcMeshNode->CalculateNormals();

	m_pcMeshNode->UpdateVertices();
	m_pcMeshNode->UpdateNormals();

}

void	CVertexAnimation::Render(bool bUseVBO)
{
	if(bUseVBO && GL_1_5_supported)
	{
		//Drawing with vbo's
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_pcMeshNode->m_nVBOVertices );
		glVertexPointer( 3, GL_FLOAT, 0,(char *) NULL);

		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_pcMeshNode->m_nVBONormals );
		glNormalPointer( GL_FLOAT, 0,(char *) NULL);

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_pcMeshNode->m_nVBOIndices);
		
		glDrawElements(GL_TRIANGLES,m_pcMeshNode->GetIndexCnt(), GL_UNSIGNED_INT, NULL);

		glPopClientAttrib();
	}
	else
	{
		// Draw simply geometry with normals 
		uint32    nIndex;
		glBegin(GL_TRIANGLES);
		for(uint32 i=0;i<m_pcMeshNode->GetIndexCnt();i++)
		{
			nIndex = m_pcMeshNode->GetVertIndex(i);
			glNormal3fv(m_pcMeshNode->m_pvNormals[nIndex]);
			glVertex3fv(m_pcMeshNode->m_pvVertices[nIndex]);
		}
		glEnd();
	}

}