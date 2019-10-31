
#ifndef MPMATH_UTILS_H
#define MPMATH_UTILS_H

#include "mpVec2.h"
#include "mpVec3.h"
#include "mpMatrix3x3.h"
#include <vector>

namespace mp {

    template<typename T>
    mp::Matrix3x3<T>   makeSkewSymmetric(const mp::Vec3<T>& v)
    {
        return mp::Matrix3x3<T>( T(0),-v[2], v[1],
                                 v[2], T(0),-v[0],
                                -v[1], v[0], T(0));
    }

    template<typename T>
    void orthonormalize(mp::Matrix3x3<T>& m)
    {
        //take the columns
        mp::Vec3<T> x(m.m_mat[0][0],m.m_mat[1][0],m.m_mat[2][0]);
        mp::Vec3<T> y(m.m_mat[0][1],m.m_mat[1][1],m.m_mat[2][1]);
        mp::Vec3<T> z;

        //apply Gram-Schmidt, sort of
        x.normalize();
        z = cross(x,y);
        z.normalize();
        y = cross(z,x);
        y.normalize();

        //set orthogonalized basis as columns
        m.m_mat[0][0] = x[0]; m.m_mat[0][1] = y[0]; m.m_mat[0][2] = z[0];
        m.m_mat[1][0] = x[1]; m.m_mat[1][1] = y[1]; m.m_mat[1][2] = z[1];
        m.m_mat[2][0] = x[2]; m.m_mat[2][1] = y[2]; m.m_mat[2][2] = z[2];
    }

} //namespace mp

template<typename T>
mp::Vec3<T> operator*(const mp::Matrix3x3<T>& m, const mp::Vec3<T>& v)
{
    return mp::Vec3<T>(m[0][0]*v[0]+m[0][1]*v[1]+m[0][2]*v[2],
                       m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]*v[2],
                       m[2][0]*v[0]+m[2][1]*v[1]+m[2][2]*v[2]);
}

#endif //MPMATH_UTILS_H
