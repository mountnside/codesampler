
#ifndef MPMATH_MATRIX3X3_H
#define MPMATH_MATRIX3X3_H

#include <cassert>
#include <utility>

namespace mp {

template<typename T>
class Matrix3x3 {
public:

    Matrix3x3()
    {
        m_mat[0][0] = T(0);
        m_mat[1][0] = T(0);
        m_mat[2][0] = T(0);
        m_mat[0][1] = T(0);
        m_mat[1][1] = T(0);
        m_mat[2][1] = T(0);
        m_mat[0][2] = T(0);
        m_mat[1][2] = T(0);
        m_mat[2][2] = T(0);
    }

    //this constructor takes a buffer of values
    //in row-major (i.e. C/C++ major) format
    Matrix3x3(T* ptr)
    {
        m_mat[0][0] = *ptr++;
        m_mat[0][1] = *ptr++;
        m_mat[0][2] = *ptr++;
        m_mat[1][0] = *ptr++;
        m_mat[1][1] = *ptr++;
        m_mat[1][2] = *ptr++;
        m_mat[2][0] = *ptr++;
        m_mat[2][1] = *ptr++;
        m_mat[2][2] = *ptr;
    }

    Matrix3x3(T a00, T a01, T a02,
              T a10, T a11, T a12,
              T a20, T a21, T a22)
    {
        m_mat[0][0] = a00;
        m_mat[0][1] = a01;
        m_mat[0][2] = a02;
        m_mat[1][0] = a10;
        m_mat[1][1] = a11;
        m_mat[1][2] = a12;
        m_mat[2][0] = a20;
        m_mat[2][1] = a21;
        m_mat[2][2] = a22;
    }

    Matrix3x3(const Matrix3x3& m)
    {
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] = m.m_mat[i][j];
    }

    Matrix3x3& transpose()
    {
        std::swap(m_mat[0][1],m_mat[1][0]);
        std::swap(m_mat[0][2],m_mat[2][0]);
        std::swap(m_mat[2][1],m_mat[1][2]);
        return *this;
    }

    Matrix3x3& makeIdentity()
    {
        m_mat[0][0] = T(1);
        m_mat[1][0] = T(0);
        m_mat[2][0] = T(0);
        m_mat[0][1] = T(0);
        m_mat[1][1] = T(1);
        m_mat[2][1] = T(0);
        m_mat[0][2] = T(0);
        m_mat[1][2] = T(0);
        m_mat[2][2] = T(1);
        return *this;
    }

    //returns false if the matrix is singular
    bool invert()
    {
        //compute determinant
        T	d = m_mat[0][0]*m_mat[1][1]*m_mat[2][2] - 
                m_mat[0][0]*m_mat[2][1]*m_mat[1][2] + 
                m_mat[1][0]*m_mat[2][1]*m_mat[0][2] - 
                m_mat[1][0]*m_mat[0][1]*m_mat[2][2] + 
                m_mat[2][0]*m_mat[0][1]*m_mat[1][2] - 
                m_mat[2][0]*m_mat[1][1]*m_mat[0][2];

        //if zero (or close to), the matrix is singular
        if (d == T(0))
        {
            return false;
        }

        Matrix3x3 tmp(*this);

        m_mat[0][0] = (tmp.m_mat[1][1]*tmp.m_mat[2][2]-tmp.m_mat[1][2]*tmp.m_mat[2][1])/d;
        m_mat[0][1] = -(tmp.m_mat[0][1]*tmp.m_mat[2][2]-tmp.m_mat[0][2]*tmp.m_mat[2][1])/d;
        m_mat[0][2] = (tmp.m_mat[0][1]*tmp.m_mat[1][2]-tmp.m_mat[0][2]*tmp.m_mat[1][1])/d;
        m_mat[1][0] = -(tmp.m_mat[1][0]*tmp.m_mat[2][2]-tmp.m_mat[1][2]*tmp.m_mat[2][0])/d;
        m_mat[1][1] = (tmp.m_mat[0][0]*tmp.m_mat[2][2]-tmp.m_mat[0][2]*tmp.m_mat[2][0])/d;
        m_mat[1][2] = -(tmp.m_mat[0][0]*tmp.m_mat[1][2]-tmp.m_mat[0][2]*tmp.m_mat[1][0])/d;
        m_mat[2][0] = (tmp.m_mat[1][0]*tmp.m_mat[2][1]-tmp.m_mat[1][1]*tmp.m_mat[2][0])/d;
        m_mat[2][1] = -(tmp.m_mat[0][0]*tmp.m_mat[2][1]-tmp.m_mat[0][1]*tmp.m_mat[2][0])/d;
        m_mat[2][2] = (tmp.m_mat[0][0]*tmp.m_mat[1][1]-tmp.m_mat[0][1]*tmp.m_mat[1][0])/d;	

        return true;
    }

    Matrix3x3& postMultiply(const Matrix3x3& m)
    {
        if (this == &m) {
            Matrix3x3 t(m);
            postMultiply(t);
            return *this;
        }

        T r0, r1, r2;

        int i;
        for (i=0;i<3;i++) {

            r0 = m_mat[i][0] * m.m_mat[0][0] + 
                 m_mat[i][1] * m.m_mat[1][0] +
                 m_mat[i][2] * m.m_mat[2][0];

            r1 = m_mat[i][0] * m.m_mat[0][1] + 
                 m_mat[i][1] * m.m_mat[1][1] +
                 m_mat[i][2] * m.m_mat[2][1];
           
            r2 = m_mat[i][0] * m.m_mat[0][2] + 
                 m_mat[i][1] * m.m_mat[1][2] +
                 m_mat[i][2] * m.m_mat[2][2];

            m_mat[i][0] = r0;
            m_mat[i][1] = r1;
            m_mat[i][2] = r2;

        }
        return *this;
    }

    Matrix3x3& preMultiply(const Matrix3x3& m)
    {
        if (this == &m) {
            Matrix3x3 t(m);
            preMultiply(t);
            return *this;
        }

        T c0, c1, c2;

        for (int j = 0; j<3; j++) {
            c0 = m.m_mat[0][0] * m_mat[0][j] +
                 m.m_mat[0][1] * m_mat[1][j] +
                 m.m_mat[0][2] * m_mat[2][j];

            c1 = m.m_mat[1][0] * m_mat[0][j] +
                 m.m_mat[1][1] * m_mat[1][j] +
                 m.m_mat[1][2] * m_mat[2][j];

            c2 = m.m_mat[2][0] * m_mat[0][j] +
                 m.m_mat[2][1] * m_mat[1][j] +
                 m.m_mat[2][2] * m_mat[2][j];

            m_mat[0][j] = c0;
            m_mat[1][j] = c1;
            m_mat[2][j] = c2;
        }

        return *this;
    }

    Matrix3x3& operator=(const Matrix3x3& m)
    {
        if(this == &m) return *this;

        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] = m.m_mat[i][j];

        return *this;
    }

    Matrix3x3& operator+=( const Matrix3x3 &m )
    {
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] += m.m_mat[i][j];
        return *this;
    }

    //post-multiply
    Matrix3x3 &operator*=( const Matrix3x3 &m)
    {
        return this->postMultiply(m);
    }

    Matrix3x3&  set(T a00, T a01, T a02,
                    T a10, T a11, T a12,
                    T a20, T a21, T a22)
    {
        m_mat[0][0] = a00;
        m_mat[0][1] = a01;
        m_mat[0][2] = a02;
        m_mat[1][0] = a10;
        m_mat[1][1] = a11;
        m_mat[1][2] = a12;
        m_mat[2][0] = a20;
        m_mat[2][1] = a21;
        m_mat[2][2] = a22;
        return *this;
    }


    Matrix3x3& operator+=( T value )
    {
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] += value;
        return *this;
    }

    Matrix3x3& operator-=( T value )
    {
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] -= value;
        return *this;
    }

    Matrix3x3& operator*=( T value )
    {
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                m_mat[i][j] *= value;
        return *this;
    }

    /**
    * Returns a pointer to the row of the matrix associated with the given 
    * index.
    *
    * @param i The index of the desired row, where 0 is the location of the 
    * first row, 1 is the location of the second row, etc.
    */
    T *operator [](int i) { return(&m_mat[i][0]); }

    /**
    * Returns a pointer to the row of the matrix associated with the given 
    * index.
    *
    * @param i The index of the desired row, where 0 is the location of the 
    * first row, 1 is the location of the second row, etc.
    */
    const T *operator [](int i) const { return(&m_mat[i][0]); }

    //it is open to debate whether encapsulation
    //is necessary here.
    T  m_mat[3][3];
};

} //namespace mp

template<typename T1, typename T2>
inline mp::Matrix3x3<T2> operator+(T1 value, const mp::Matrix3x3<T2>& m)
{
    return mp::Matrix3x3<T2>(m.m_mat[0][0]+value,m.m_mat[0][1]+value,m.m_mat[0][2]+value,
                             m.m_mat[1][0]+value,m.m_mat[1][1]+value,m.m_mat[1][2]+value,
                             m.m_mat[2][0]+value,m.m_mat[2][1]+value,m.m_mat[2][2]+value);
}

template<typename T1, typename T2>
inline mp::Matrix3x3<T1> operator+(const mp::Matrix3x3<T1>& m, T2 t)
{
    return mp::Matrix3x3<T1>(m.m_mat[0][0]+value,m.m_mat[0][1]+value,m.m_mat[0][2]+value,
                             m.m_mat[1][0]+value,m.m_mat[1][1]+value,m.m_mat[1][2]+value,
                             m.m_mat[2][0]+value,m.m_mat[2][1]+value,m.m_mat[2][2]+value);
}

template<typename T1, typename T2>
inline mp::Matrix3x3<T2> operator-(T1 value, const mp::Matrix3x3<T2>& m)
{
    return mp::Matrix3x3<T2>(-m.m_mat[0][0]+value,-m.m_mat[0][1]+value,-m.m_mat[0][2]+value,
                             -m.m_mat[1][0]+value,-m.m_mat[1][1]+value,-m.m_mat[1][2]+value,
                             -m.m_mat[2][0]+value,-m.m_mat[2][1]+value,-m.m_mat[2][2]+value);
}

template<typename T1, typename T2>
inline mp::Matrix3x3<T1> operator-(const mp::Matrix3x3<T1>& m, T2 t)
{
    return mp::Matrix3x3<T1>(m.m_mat[0][0]-value,m.m_mat[0][1]-value,m.m_mat[0][2]-value,
                             m.m_mat[1][0]-value,m.m_mat[1][1]-value,m.m_mat[1][2]-value,
                             m.m_mat[2][0]-value,m.m_mat[2][1]-value,m.m_mat[2][2]-value);
}

template<typename T1, typename T2>
inline mp::Matrix3x3<T2> operator*(T1 value, const mp::Matrix3x3<T2>& m)
{
    return mp::Matrix3x3<T2>(m.m_mat[0][0]*value,m.m_mat[0][1]*value,m.m_mat[0][2]*value,
                             m.m_mat[1][0]*value,m.m_mat[1][1]*value,m.m_mat[1][2]*value,
                             m.m_mat[2][0]*value,m.m_mat[2][1]*value,m.m_mat[2][2]*value);
}

template<typename T1, typename T2>
inline mp::Matrix3x3<T1> operator*(const mp::Matrix3x3<T1>& m, T2 t)
{
    return mp::Matrix3x3<T1>(m.m_mat[0][0]*value,m.m_mat[0][1]*value,m.m_mat[0][2]*value,
                             m.m_mat[1][0]*value,m.m_mat[1][1]*value,m.m_mat[1][2]*value,
                             m.m_mat[2][0]*value,m.m_mat[2][1]*value,m.m_mat[2][2]*value);
}

template<typename T>
inline mp::Matrix3x3<T> operator+(const mp::Matrix3x3<T>& m1, const mp::Matrix3x3<T>& m2)
{
    return mp::Matrix3x3<T>(m1.m_mat[0][0]+m2.m_mat[0][0],
                            m1.m_mat[0][1]+m2.m_mat[0][1],
                            m1.m_mat[0][2]+m2.m_mat[0][2],
                            m1.m_mat[1][0]+m2.m_mat[1][0],
                            m1.m_mat[1][1]+m2.m_mat[1][1],
                            m1.m_mat[1][2]+m2.m_mat[1][2],
                            m1.m_mat[2][0]+m2.m_mat[2][0],
                            m1.m_mat[2][1]+m2.m_mat[2][1],
                            m1.m_mat[2][2]+m2.m_mat[2][2]);
}

template<typename T>
inline mp::Matrix3x3<T> operator*(const mp::Matrix3x3<T>& m1, const mp::Matrix3x3<T>& m2)
{
    return mp::Matrix3x3<T>(m1.m_mat[0][0]*m2.m_mat[0][0]+
                            m1.m_mat[0][1]*m2.m_mat[1][0]+
                            m1.m_mat[0][2]*m2.m_mat[2][0], //a00
                            m1.m_mat[0][0]*m2.m_mat[0][1]+
                            m1.m_mat[0][1]*m2.m_mat[1][1]+
                            m1.m_mat[0][2]*m2.m_mat[2][1], //a01
                            m1.m_mat[0][0]*m2.m_mat[0][2]+
                            m1.m_mat[0][1]*m2.m_mat[1][2]+
                            m1.m_mat[0][2]*m2.m_mat[2][2], //a02
                            m1.m_mat[1][0]*m2.m_mat[0][0]+
                            m1.m_mat[1][1]*m2.m_mat[1][0]+
                            m1.m_mat[1][2]*m2.m_mat[2][0], //a10
                            m1.m_mat[1][0]*m2.m_mat[0][1]+
                            m1.m_mat[1][1]*m2.m_mat[1][1]+
                            m1.m_mat[1][2]*m2.m_mat[2][1], //a11
                            m1.m_mat[1][0]*m2.m_mat[0][2]+
                            m1.m_mat[1][1]*m2.m_mat[1][2]+
                            m1.m_mat[1][2]*m2.m_mat[2][2], //a12
                            m1.m_mat[2][0]*m2.m_mat[0][0]+
                            m1.m_mat[2][1]*m2.m_mat[1][0]+
                            m1.m_mat[2][2]*m2.m_mat[2][0], //a20
                            m1.m_mat[2][0]*m2.m_mat[0][1]+
                            m1.m_mat[2][1]*m2.m_mat[1][1]+
                            m1.m_mat[2][2]*m2.m_mat[2][1], //a21
                            m1.m_mat[2][0]*m2.m_mat[0][2]+
                            m1.m_mat[2][1]*m2.m_mat[1][2]+
                            m1.m_mat[2][2]*m2.m_mat[2][2]);//a22
}

#endif //MPMATH_MATRIX3X3_H
