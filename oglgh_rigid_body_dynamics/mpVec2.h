
#ifndef MPMTAH_VEC2_H
#define MPMTAH_VEC2_H

#include <cstdio>

namespace mp {

template <class Type>
class Vec2 {

public:

    Type m_vec[2];

    /**
     * Creates a vector w/ all values initialized to zero.
     */
    Vec2() { m_vec[0] = m_vec[1] = Type(0); }

    /**
     * Creates a vector initialized w/ the given values.
     *
     * @param x The value of the first component.
     * @param y The value of the second component.
     */
    Vec2(Type x, Type y) { m_vec[0] = x; m_vec[1] = y; }

    /**
     * Creates a vector which is a copy of @e src.
     *
     * @param src The instance to be copied.
     */
    Vec2(const Vec2 &src)
    {
        m_vec[0] = src.m_vec[0]; m_vec[1] = src.m_vec[1];
    }

    /**
     * Creates a vector initialized with the values of the given array.
     *
     * @param a An array which contains the desired values..
     */
    Vec2(const Type a[2]) { set(a); }

    /**
     * Destructor.
     */
    ~Vec2(){}

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
     * Sets the component values of the vector to the values stored in the
     * given variables.
     *
     * @param x The desired value for the first component.
     * @param y The desired value for the second component.
     */
    inline void set(Type x, Type y) { m_vec[0] = x; m_vec[1] = y; }

    /**
     * Gets the component values of the vector and stores them into the given
     * variables.
     *
     * @param x The variable to store the first component’s value into.
     * @param y The variable to store the second component’s value into.
     */
    inline void get(Type* x, Type* y) const { *x = m_vec[0]; *y = m_vec[1]; }

    /**
     * Sets the component values of the vector to the values stored in the
     * given array.
     *
     * @param a An array which contains the desired values..
     */
    inline void set(const Type a[2]) { m_vec[0] = a[0]; m_vec[1] = a[1];}

    /**
     * Gets the component values of the vector and stores them into the given
     * array.
     * 
     * @param a The array to store the component values into.
     */
    inline void get(Type a[2]) const { a[0] = m_vec[0]; a[1] = m_vec[1]; }

    /**
     * Makes this instance a copy of @e src.
     *
     * @param src The instance to be copied.
     * @return A reference to the instance being operated on.
     */
    Vec2& operator =(const Vec2 &src) { 
        
        m_vec[0] = src.m_vec[0]; m_vec[1] = src.m_vec[1]; 
        return *this; 
    }

    /**
     * Sets this instance equal to the sum of itself and @e v (i.e. vec += v).
     *
     * @param v The instance to be added.
     * @return A reference to the instance being operated on.
     */
    Vec2& operator +=(const Vec2 &v) 
    { 
        m_vec[0] += v.m_vec[0]; m_vec[1] += v.m_vec[1]; 
        return *this; 
    }

    /**
     * Sets this instance equal to the difference between itself and @e v
     * (i.e. vec -= v).
     *
     * @param v The instance to be subtracted.
     * @return A reference to the instance being operated on.
     */
    Vec2& operator -=(const Vec2 &v) { 

        m_vec[0] -= v.m_vec[0]; m_vec[1] -= v.m_vec[1]; 
        return *this; 
    }

    /**
     * Sets this instance equal to a scaled version of itself (i.e. vec *= s).
     *
     * @param s The scale factor.
     * @return A reference to the instance being operated on.
     */
    Vec2& operator *=(const Type s) 
    {
        m_vec[0] *= s; m_vec[1] *= s; 
        return *this; 
    }

    /**
     * Returns true if @e lhs is equal to @e rhs, false otherwise.
     *
     * @param lhs The vector on the left hand side of the operator.
     * @param rhs The vector on the right hand side of the operator.
     */
    friend inline bool operator==(const Vec2 &lhs, const Vec2& rhs)
    {
        return(lhs.m_vec[0] == rhs.m_vec[0] && lhs.m_vec[1] == rhs.m_vec[1]);
    }

    /**
     * Returns true if @e lhs is not equal to @e rhs, false otherwise.
     *
     * @param lhs The vector on the left hand side of the operator.
     * @param rhs The vector on the right hand side of the operator.
     */
    friend inline bool operator!=(const Vec2 &lhs, const Vec2& rhs)
    {
        return(!(lhs == rhs));
    }

    /**
     * Sets the instance equal to the negation of @e v (i.e. vec = -v).
     *
     * @param v The instance to be negated.
     */
    inline void negate(const Vec2 &v)
    {
        m_vec[0] = -v.m_vec[0];
        m_vec[1] = -v.m_vec[1];
    }

};

//a little helper function
template<typename T>
void outputPoint(const mp::Vec2<T> & pt, FILE* fout)
{
    fprintf(fout,"%g %g\n",pt[0],pt[1]);
}

template<typename T>
inline T dot(const Vec2<T>& v1, const Vec2<T>& v2)
{
    return (v1[0]*v2[0]+v1[1]*v2[1]);
}

} //namespace mp

#endif //MPMTAH_VEC2_H
