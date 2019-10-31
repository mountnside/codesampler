//-----------------------------------------------------------------------------
//           Name: shadow_volume.h
//         Author: Mostly from a DX9 SDK sample
//  Last Modified: 03/03/04
//    Description: Simple class for grouping together as that's needed to 
//                 create and use a shadow volume.
//
// Note: The buildShadowVolume() method contains a data type called MeshVertex,
//       which is basically hard-coded to work with my sample's teapot mesh.
//       If you want to use another mesh instead of the teapot, you'll need to
//       find out what the FVF layout is for its vertex type and substitute it
//       there for MeshVertex.
//-----------------------------------------------------------------------------

class ShadowVolume
{
public:

    void    reset() { m_dwNumVertices = 0L; }
    HRESULT buildShadowVolume( LPD3DXMESH pObject, D3DXVECTOR3 vLight );
    HRESULT render( LPDIRECT3DDEVICE8 pd3dDevice );

private:

	void addEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 );

	D3DXVECTOR3 m_pVertices[32000]; // Vertex data for rendering shadow volume
    DWORD       m_dwNumVertices;
};

struct ShadowVertex
{
    D3DXVECTOR4 p;
    D3DCOLOR    color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZRHW | D3DFVF_DIFFUSE
	};
};

//-----------------------------------------------------------------------------
// Name: render
// Desc:
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::render( LPDIRECT3DDEVICE8 pd3dDevice )
{
    pd3dDevice->SetVertexShader( D3DFVF_XYZ );

    return pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_dwNumVertices/3,
                                        m_pVertices, sizeof(D3DXVECTOR3) );
}

//-----------------------------------------------------------------------------
// Name: buildShadowVolume()
// Desc: Takes a mesh as input, and uses it to build a shadow volume. The
//       technique used considers each triangle of the mesh, and adds it's
//       edges to a temporary list. The edge list is maintained, such that
//       only silohuette edges are kept. Finally, the silohuette edges are
//       extruded to make the shadow volume vertex list.
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::buildShadowVolume( LPD3DXMESH pMesh, D3DXVECTOR3 vLight )
{
	// Note: the MeshVertex format depends on the FVF of the mesh
	struct MeshVertex { D3DXVECTOR3 p, n; }; // Works fine for teapot, but won't do for all meshes!

    MeshVertex *pVertices;
    WORD       *pIndices;

    // Lock the geometry buffers
    pMesh->LockVertexBuffer( 0L, (BYTE**)&pVertices );
    pMesh->LockIndexBuffer( 0L, (BYTE**)&pIndices );
    DWORD dwNumFaces    = pMesh->GetNumFaces();

    // Allocate a temporary edge list
    WORD *pEdges = new WORD[dwNumFaces*6];

    if( pEdges == NULL )
    {
        pMesh->UnlockVertexBuffer();
        pMesh->UnlockIndexBuffer();
        return E_OUTOFMEMORY;
    }
	
    DWORD dwNumEdges = 0;

    // For each face
    for( DWORD i = 0; i < dwNumFaces; ++i )
    {
        WORD wFace0 = pIndices[3*i+0];
        WORD wFace1 = pIndices[3*i+1];
        WORD wFace2 = pIndices[3*i+2];

        D3DXVECTOR3 v0 = pVertices[wFace0].p;
        D3DXVECTOR3 v1 = pVertices[wFace1].p;
        D3DXVECTOR3 v2 = pVertices[wFace2].p;

        // Transform vertices or transform light?
        D3DXVECTOR3 vCross1(v2-v1);
        D3DXVECTOR3 vCross2(v1-v0);
        D3DXVECTOR3 vNormal;
        D3DXVec3Cross( &vNormal, &vCross1, &vCross2 );

        if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f )
        {
            addEdge( pEdges, dwNumEdges, wFace0, wFace1 );
            addEdge( pEdges, dwNumEdges, wFace1, wFace2 );
            addEdge( pEdges, dwNumEdges, wFace2, wFace0 );
        }
    }

    for( i = 0; i < dwNumEdges; ++i )
    {
        D3DXVECTOR3 v1 = pVertices[pEdges[2*i+0]].p;
        D3DXVECTOR3 v2 = pVertices[pEdges[2*i+1]].p;
        D3DXVECTOR3 v3 = v1 - vLight*10;
        D3DXVECTOR3 v4 = v2 - vLight*10;

        // Add a quad (two triangles) to the vertex list
        m_pVertices[m_dwNumVertices++] = v1;
        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v3;

        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v4;
        m_pVertices[m_dwNumVertices++] = v3;
    }

    // Delete the temporary edge list
    delete[] pEdges;

    // Unlock the geometry buffers
    pMesh->UnlockVertexBuffer();
    pMesh->UnlockIndexBuffer();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: addEdge()
// Desc: Adds an edge to a list of silohuette edges of a shadow volume.
//-----------------------------------------------------------------------------
void ShadowVolume::addEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
    // Remove interior edges (which appear in the list twice)
    for( DWORD i = 0; i < dwNumEdges; ++i )
    {
        if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
            ( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
        {
            if( dwNumEdges > 1 )
            {
                pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
                pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
            }

            --dwNumEdges;
            return;
        }
    }

    pEdges[2*dwNumEdges+0] = v0;
    pEdges[2*dwNumEdges+1] = v1;
    dwNumEdges++;
}