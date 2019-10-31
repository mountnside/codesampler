//-----------------------------------------------------------------------------
//         Author: Tomas Dirvanauskas (aka Tactic)
//    Description: Loads vertex animation mesh (exported from MAX)
//				   Shows how to use VBOs to quickly send data to GPU and render it
//				   Three buffers are created - for vertices,normals and indices
//				   Vertices and normals are changed every frame
//				   Indices are loaded only during mesh loading
//-----------------------------------------------------------------------------

#ifndef __VERTEX_ANIMATION__
#define __VERTEX_ANIMATION__

#include <vector>
#include "Types.h"
#include "Vector3D.h"
#include "InterpolationTable.h"

typedef unsigned int GLuint;

class CVertexAnimation
{
protected:
	class CMeshNode
	{
		friend class CVertexAnimation;
	private:
		std::vector<Vector3D>			m_pvInitialVerts;		//Initial vertex data (used when no animation found)
		
		std::vector<uint32>				m_pnIndices;			//Face indices

		InterpolationTable**			m_pvAnimTable;			//m_pvAnimTable[Vertex]->Lerp(t from 0.0f to 1.0f) 
																//Animation is here

		std::vector<Vector3D>			m_pvNormals;			//Storage for calculated normals
		std::vector<Vector3D>			m_pvVertices;			//Storage for calculated vertices

		uint32							m_nAnimCnt;				//Max Frame Cnt

		float							m_fDeltaTimeModifier;	//Multiply this sucker by your delta time :)

	private:
		//Id's to vbos
		GLuint						m_nVBOVertices;
		GLuint						m_nVBONormals;
		GLuint						m_nVBOIndices;
	public:	
		CMeshNode();
		~CMeshNode();
		//Vertex cnt in model
		inline	uint32				GetVertexCnt(){return m_pvInitialVerts.size();}
		//Index cnt in model
		inline	uint32				GetIndexCnt(){return m_pnIndices.size();}
		//How many frames do we have
		inline	uint32				GetFrameCnt	(){return m_nAnimCnt;}
		//Return initial vertex (as with t=0.0f)
		inline  Vector3D			GetIniVert  (uint32 n){return m_pvInitialVerts[n];}
		//Returns index to vertex
		inline  uint32				GetVertIndex(uint32 n){return m_pnIndices[n];}
		//Interpolates vertex
		inline	Vector3D			LerpVert	(uint32 nVertex,float t){return m_pvAnimTable[nVertex]->Lerp(t);}
		//Returs delta time modifier, so the animation speed would the same as in Max :)
		inline	float				GetDeltaTimeMod()const{return m_fDeltaTimeModifier;}
		
		//Calculates vertices with value t
		void						CalculateVertices(float t);
		//Calculate normals by using last calculated vertices
		void						CalculateNormals ();

		//Create VBO for vertices,normals and indices
		void						CreateVBOs();
		//Delete them all
		void						DeleteVBOs();
		//Upload vertices to VBO
		void						UpdateVertices();
		//Upload normals to VBO
		void						UpdateNormals ();
		//Note we don't need to upload indices to VBO, cause they're always the same
	}*m_pcMeshNode; 


	float	m_fProgress;
public:
	CVertexAnimation(const char *strFileName);
	virtual ~CVertexAnimation();
	//Just some info
	uint32  GetVertexCount()const{return m_pcMeshNode->GetVertexCnt();}

	void	Update(float fDeltaTime);

	void	Render(bool bUseVBO);

};

#endif
