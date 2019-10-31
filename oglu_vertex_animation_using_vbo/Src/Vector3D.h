#ifndef	__VECTOR3D__
#define	__VECTOR3D__

#include <math.h>

//Basic Vector class

class	Vector3D
{
public:
	float	x, y, z;

	Vector3D () {}
	Vector3D ( float px, float py, float pz )
	{
		x = px;
		y = py;
		z = pz;
	}

	Vector3D ( const Vector3D& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector3D& operator = ( const Vector3D& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	Vector3D operator + () const
	{
		return *this;
	}

	Vector3D operator - () const
	{
		return Vector3D ( -x, -y, -z );
	}

	Vector3D& operator += ( const Vector3D& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	Vector3D& operator -= ( const Vector3D& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	Vector3D& operator *= ( const Vector3D& v )
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;

		return *this;
	}

	Vector3D& operator *= ( float f )
	{
		x *= f;
		y *= f;
		z *= f;

		return *this;
	}

	Vector3D& operator /= ( const Vector3D& v )
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;

		return *this;
	}

	Vector3D& operator /= ( float f )
	{
		x /= f;
		y /= f;
		z /= f;

		return *this;
	}

	operator float * ()
	{
		return &x;
	}

	operator const float * () const
	{
		return &x;
	}


	bool	operator == ( const Vector3D& v ) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool	operator != ( const Vector3D& v ) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	float	length () const
	{
		return (float) sqrt ( x * x + y * y + z * z );
	}

    float   lengthSq () const
    {
        return x * x + y * y + z * z;
    }

	Vector3D&	normalize ()
	{
        float len=length ();
        if(len==0.0f)len=1.0f;
		return (*this) /= len;
	}

	friend Vector3D operator + ( const Vector3D&, const Vector3D& );
	friend Vector3D operator - ( const Vector3D&, const Vector3D& );
	friend Vector3D operator * ( const Vector3D&, const Vector3D& );
	friend Vector3D operator * ( float,           const Vector3D& );
	friend Vector3D operator * ( const Vector3D&, float );
	friend Vector3D operator / ( const Vector3D&, float );
	friend Vector3D operator / ( const Vector3D&, const Vector3D& );
	friend float    operator & ( const Vector3D&, const Vector3D& );
	friend Vector3D operator ^ ( const Vector3D&, const Vector3D& );

};

inline Vector3D operator + ( const Vector3D& u, const Vector3D& v )
{
	return Vector3D ( u.x + v.x, u.y + v.y, u.z + v.z );
}

inline Vector3D operator - ( const Vector3D& u, const Vector3D& v )
{
	return Vector3D ( u.x - v.x, u.y - v.y, u.z - v.z );
}

inline Vector3D operator * ( const Vector3D& u, const Vector3D& v )
{
	return Vector3D ( u.x*v.x, u.y*v.y, u.z * v.z );
}

inline Vector3D operator * ( const Vector3D& v, float a )
{
	return Vector3D ( v.x*a, v.y*a, v.z*a );
}

inline Vector3D operator * ( float a, const Vector3D& v )
{
	return Vector3D ( v.x*a, v.y*a, v.z*a );
}

inline Vector3D operator / ( const Vector3D& u, const Vector3D& v )
{
	return Vector3D ( u.x/v.x, u.y/v.y, u.z/v.z );
}

inline Vector3D operator / ( const Vector3D& v, float a )
{
	return Vector3D ( v.x/a, v.y/a, v.z/a );
}

inline Vector3D operator / ( float a, const Vector3D& v )
{
	return Vector3D ( a / v.x, a / v.y, a / v.z );
}

inline float Dot ( const Vector3D& u, const Vector3D& v )
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

inline float operator & ( const Vector3D& u, const Vector3D& v )
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

inline Vector3D Cross ( const Vector3D& u, const Vector3D& v )
{
	return Vector3D (u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x);
}

inline	Vector3D lerp ( const Vector3D& a, const Vector3D& b, float t )
{
	return a + t * (b - a);
}

#endif
