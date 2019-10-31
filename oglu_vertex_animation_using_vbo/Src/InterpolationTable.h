//-----------------------------------------------------------------------------
//    Description: Simple Vector interpolation Class
//-----------------------------------------------------------------------------

#ifndef INTERPOLATIONTABLE_H
#define INTERPOLATIONTABLE_H

#include "Vector3D.h"

//Simple class for interpolating vectors

class InterpolationTable
{
private:
    int		  m_nMaxPoints;

    Vector3D *m_pvPoints;                               // First point at x = xMin, last at x = xMax

    float	  m_fMin, m_fMax,m_fSize;					// Interpolating value

    float	  m_fScale;									// Internal use
public: 
	
    InterpolationTable();
	~InterpolationTable() { FreeTable(); }

	void		CreateTable(int nMaxPoints, Vector3D *pvPoints, float nMin, float nMax);

    void		FreeTable();

    Vector3D	Lerp( float x);

   
};



#endif

