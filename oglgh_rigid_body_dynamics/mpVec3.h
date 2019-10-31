#ifndef MPMTAH_VEC3_H
#define MPMTAH_VEC3_H
#include <cstdio>

namespace mp {

template <class Type>
class Vec3 {

public:

    Type m_vec[3];

    /**
    * Creates a vector w/ all values initialized to zero.
    */
    Vec3(){ m_vec[0] = m_vec[1] = m_vec[2] = Type(0); }

    /**
    * Creates a vector initialized w/ the given values.
    *
    * @param x The value of the first component.
    * @param y The value of the second component.
    * @param z The value of the third component.
    */
    Vec3(Type x, Type y, Type z)
    {
        m_vec[0] = x; m_vec[1] = y; m_vec[2] = z;
    }

    /**
    * Creates a vector which is a copy of @e src.
    *
    * @param src The instance to be copied.
    */
    Vec3(const Vec3 &src) 
    {
        m_vec[0] = src.m_vec[0]; m_vec[1] = src.m_vec[1];
        m_vec[2] = src.m_vec[2];
    }

    /**
    * Creates a vector initialized with the values of the given array.
    *
    * @param a An array which contains the desired values..
    */
    Vec3(const Type a[3]) { set(a); }

    /**
    * Destructor.
    */
    ~Vec3(){}

    /**
    * Returns the value of the component associated with the given index.
    *
    * @param index The index of the desired component, where 0 is the index
    * of the first component, 1 is the index of the second component, etc..
    */
    const Type &operator [](int index) const { return(m_vec[index]); }

    /**
    * Returns the value of the component associated with the given index.
    *
    * @param index The index of the desired component, where 0 is the index
    * of the first component, 1 is the index of the second component, etc..
    */
    Type &operator [](int index) { return(m_vec[index]); }

    /**
    * Makes this instance a copy of @e src.
    *
    * @param src The instance to be copied.
    * @return A reference to the instance being operated on.
    */
    Vec3& operator=(const Vec3 &src) 
    { 
        m_vec[0] = src.m_vec[0]; m_vec[1] = src.m_vec[1];  m_vec[2] = src.m_vec[2];
        return *this; 
    }

    /**
    * Sets the component values of the vector to the values stored in the
    * given variables.
    *
    * @param x The desired value for the first component.
    * @param y The desired value for the second component.
    * @param z The desired value for the third component.
    */
    inline void set(Type x, Type y, Type z) {
        m_vec[0] = x; m_vec[1] = y; m_vec[2] = z;
    }

    /**
    * Gets the component values of the vector and stores them into the given
    * variables.
    *
    * @param x The variable to store the first component’s value into.
    * @param y The variable to store the second component’s value into.
    * @param z The variable to store the third component’s value into.
    */
    inline void get(Type* x, Type* y, Type* z) const
    {
        *x=m_vec[0]; *y=m_vec[1]; *z=m_vec[2];
    }

    /**
    * Sets the component values of the vector to the values stored in the
    * given array.
    *
    * @param a An array which contains the desired values..
    */
    inline void set(const Type a[3]) {
        m_vec[0] = a[0]; m_vec[1] = a[1]; m_vec[2] = a[2];
    }

    /**
    * Gets the component values of the vector and stores them into the given
    * array.
    * 
    * @param a The array to store the component values into.
    */
    inline void get(Type a[3]) const 
    {
        a[0] = m_vec[0]; a[1] = m_vec[1]; a[2] = m_vec[2];
    }

    /**
    * Returns a vector which is equal to the sum of @e v1 and @e v2.  Note
    * that this function creates a temporary vector to store the results into.
    *
    * @param v1 The vector on the left hand side of the operator.
    * @param v2 The vector on the right hand side of the operator.
    */
    friend Vec3 operator +(const Vec3 &v1, const Vec3 &v2) 
    { 
        return (Vec3(v1.m_vec[0] + v2.m_vec[0], 
            v1.m_vec[1] + v2.m_vec[1],
            v1.m_vec[2] + v2.m_vec[2]));
    }

    /**
    * Sets this instance equal to the sum of itself and @e v (i.e. vec += v).
    *
    * @param v The instance to be added.
    * @return A reference to the instance being operated on.
    */
    Vec3& operator +=(const Vec3 &v) 
    { 
        m_vec[0] += v.m_vec[0]; m_vec[1] += v.m_vec[1];
        m_vec[2] += v.m_vec[2];
        return *this; 
    }

    /**
    * Sets this instance equal to the difference between itself and @e v
    * (i.e. vec -= v).
    *
    * @param v The instance to be subtracted.
    * @return A reference to the instance being operated on.
    */
    Vec3& operator -=(const Vec3 &v) 
    {
        m_vec[0] -= v.m_vec[0]; m_vec[1] -= v.m_vec[1];
        m_vec[2] -= v.m_vec[2];
        return *this; 
    }

    friend Vec3 operator -(const Vec3 &v1, const Vec3 &v2) 
    { 
        return (Vec3(v1.m_vec[0] - v2.m_vec[0], 
                v1.m_vec[1] - v2.m_vec[1],
                v1.m_vec[2] - v2.m_vec[2]));
    }

    /**
    * Returns a vector which is equal to the product of @e v1 and @e v2.
    * Note that this function creates a temporary vector to store the results
    * into.
    *
    * @param v1 The vector on the left hand side of the operator.
    * @param v2 The vector on the right hand side of the operator.
    */
    friend Vec3 operator *(const Vec3 &v1, const Vec3 &v2) 
    { 
        return (Vec3(v1[0] * v2[0], 
            v1[1] * v2[1],
            v1[2] * v2[2]));
    }

    /**
    * Returns a vector which is equal to the product of the vector @e v and
    * the scalar @e s.  Note that this function creates a temporary vector to
    * store the results into.
    *
    * @param v The vector on the left hand side of the operator.
    * @param s The scalar on the right hand side of the operator.
    */
    friend Vec3 operator * (const Vec3 &v, const Type &s) 
    { 
        return (Vec3(v[0] * s, 
            v[1] * s,
            v[2] * s));
    }

    /**
    * Returns a vector which is equal to the product of the vector @e v and
    * the scalar @e s.  Note that this function creates a temporary vector to
    * store the results into.
    *
    * @param s The scalar on the left hand side of the operator.
    * @param v The vector on the right hand side of the operator.
    */
    friend Vec3 operator * (const Type &s, const Vec3 &v) 
    { 
        return (v * s);
    }

    /**
    * Sets this instance equal to a scaled version of itself (i.e. vec *= s).
    *
    * @param s The scale factor.
    * @return A reference to the instance being operated on.
    */
    Vec3& operator *=(const Type s) 
    {
        m_vec[0] *= s; m_vec[1] *= s; m_vec[2] *= s;
        return *this; 
    }

    /**
    * Returns true if @e v1 is equal to @e v2, false otherwise.
    *
    * @param v1 The vector on the left hand side of the operator.
    * @param v2 The vector on the right hand side of the operator.
    */
    friend bool operator==(const Vec3 &v1, const Vec3& v2)
    {
        return (v1.m_vec[0] == v2.m_vec[0] && 
            v1.m_vec[1] == v2.m_vec[1] && 
            v1.m_vec[2] == v2.m_vec[2] );
    }

    /**
    * Returns true if @e v1 is not equal to @e v2, false otherwise.
    *
    * @param v1 The vector on the left hand side of the operator.
    * @param v2 The vector on the right hand side of the operator.
    */
    friend bool operator!=(const Vec3 &v1, const Vec3& v2)
    {
        return !(v1 == v2);
    }



    /**
    * Sets the instance equal to the negation of @e v (i.e. vec = -v).
    *
    * @param v The instance to be negated.
    */
    void negate(const Vec3 &v)
    {
        m_vec[0] = -v.m_vec[0];
        m_vec[1] = -v.m_vec[1];
        m_vec[2] = -v.m_vec[2];
    }

    void negate()
    {
        m_vec[0] = -m_vec[0];
        m_vec[1] = -m_vec[1];
        m_vec[2] = -m_vec[2];
    }

    Type normalize()
    {
        Type len = m_vec[0]*m_vec[0]+m_vec[1]*m_vec[1]+m_vec[2]*m_vec[2];
        if(len > Type(0))
        {
            len = std::sqrt(len);
            m_vec[0] /= len;
            m_vec[1] /= len;
            m_vec[2] /= len;
        }
        return len;
    }
};

template<typename T>
void outputPoint(const mp::Vec3<T> & pt, FILE* fout)
{
    fprintf(fout,"%g %g %g\n",pt[0],pt[1],pt[2]);
}

template<typename T>
inline T dot(const Vec3<T>& v1, const Vec3<T>& v2)
{
    return (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]);
}

template <class T>
inline Vec3<T> cross(const Vec3<T>& v1, const Vec3<T>& v2)
{
    return Vec3<T>(v1.m_vec[1] * v2.m_vec[2] - v1.m_vec[2] * v2.m_vec[1],
                   v1.m_vec[2] * v2.m_vec[0] - v1.m_vec[0] * v2.m_vec[2],
                   v1.m_vec[0] * v2.m_vec[1] - v1.m_vec[1] * v2.m_vec[0]);
}

}
#endif //MPMTAH_VEC3_H
