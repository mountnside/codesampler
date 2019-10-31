#include "InterpolationTable.h"

#include <string.h>
#include <assert.h>

InterpolationTable::InterpolationTable()
:m_nMaxPoints(0)
,m_pvPoints(NULL)
{
}

void InterpolationTable::FreeTable()
{
	m_nMaxPoints = 0;
    if (m_pvPoints) {
        delete [] m_pvPoints;
        m_pvPoints = NULL;
    }
}

void InterpolationTable::CreateTable(int nMaxPoints, Vector3D *pvPoints, float fMin, float fMax)
{
	

    FreeTable();

    assert(nMaxPoints >= 3);
	assert(fMax > fMin);

    m_nMaxPoints = nMaxPoints;

    m_pvPoints = new Vector3D[nMaxPoints];

    memcpy(m_pvPoints, pvPoints, sizeof(Vector3D) * nMaxPoints);
 
    m_fMin = fMin;

    m_fMax = fMax-0.0001f;//eps

	m_fSize=fMax-fMin;

    m_fScale = (nMaxPoints - 1) / (fMax - fMin);
}

Vector3D InterpolationTable::Lerp(float x)
{
    assert(m_pvPoints);

	//X must be in range
    if (x < m_fMin)return m_pvPoints[0];    
	if (x > m_fMax)return m_pvPoints[m_nMaxPoints - 1];   
	
    float i = (x - m_fMin) * m_fScale;

    int index = int(i);

    assert(index > -1 && index < (m_nMaxPoints - 1) && "Index was calculated incorrectly ");

    Vector3D p1 = m_pvPoints[index];      
    Vector3D p2 = m_pvPoints[index + 1];

    float t = i - index;                        

    assert(t >= 0.0 && t < 1.0 && "t range must be 0.0f..1.0f");

    return p1 + t * (p2 - p1);
}
