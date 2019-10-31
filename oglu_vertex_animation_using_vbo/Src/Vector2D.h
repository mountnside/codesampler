#ifndef	__VECTOR2D__
#define	__VECTOR2D__

#include	<math.h>

class	Vector2D
{
public:
	float	x, y;

	Vector2D () {}
	Vector2D ( float px, float py )
	{
		x = px;
		y = py;
	}

	Vector2D ( const Vector2D& v )
	{
		x = v.x;
		y = v.y;
	}

	Vector2D& operator = ( const Vector2D& v )
	{
		x = v.x;
		y = v.y;

		return *this;
	}

	Vector2D operator + () const
	{
		return *this;
	}

	Vector2D operator - () const
	{
		return Vector2D ( -x, -y );
	}

	Vector2D& operator += ( const Vector2D& v )
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	Vector2D& operator -= ( const Vector2D& v )
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}

	Vector2D& operator *= ( const Vector2D& v )
	{
		x *= v.x;
		y *= v.y;

		return *this;
	}

	Vector2D& operator *= ( float f )
	{
		x *= f;
		y *= f;

		return *this;
	}

	Vector2D& operator /= ( const Vector2D& v )
	{
		x /= v.x;
		y /= v.y;

		return *this;
	}

	Vector2D& operator /= ( float f )
	{
		x /= f;
		y /= f;

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

	int	operator == ( const Vector2D& v ) const
	{
		return x == v.x && y == v.y;
	}

	int	operator != ( const Vector2D& v ) const
	{
		return x != v.x || y != v.y;
	}

	float	length () const
	{
		return (float) sqrt ( x * x + y * y );
	}

    float   lengthSq () const
    {
	    return x * x + y * y;
    }

	Vector2D&	normalize ()
	{
		return (*this) /= length ();
	}

	friend Vector2D operator + ( const Vector2D&, const Vector2D& );
	friend Vector2D operator - ( const Vector2D&, const Vector2D& );
	friend Vector2D operator * ( const Vector2D&, const Vector2D& );
	friend Vector2D operator * ( float,           const Vector2D& );
	friend Vector2D operator * ( const Vector2D&, float );
	friend Vector2D operator / ( const Vector2D&, float );
	friend Vector2D operator / ( const Vector2D&, const Vector2D& );
	friend float    operator & ( const Vector2D&, const Vector2D& );

};

inline Vector2D operator + ( const Vector2D& u, const Vector2D& v )
{
	return Vector2D ( u.x + v.x, u.y + v.y );
}

inline Vector2D operator - ( const Vector2D& u, const Vector2D& v )
{
	return Vector2D ( u.x - v.x, u.y - v.y );
}

inline Vector2D operator * ( const Vector2D& u, const Vector2D& v )
{
	return Vector2D ( u.x*v.x, u.y*v.y );
}

inline Vector2D operator * ( const Vector2D& v, float a )
{
	return Vector2D ( v.x*a, v.y*a );
}

inline Vector2D operator * ( float a, const Vector2D& v )
{
	return Vector2D ( v.x*a, v.y*a );
}

inline Vector2D operator / ( const Vector2D& u, const Vector2D& v )
{
	return Vector2D ( u.x/v.x, u.y/v.y );
}

inline Vector2D operator / ( const Vector2D& v, float a )
{
	return Vector2D ( v.x/a, v.y/a );
}

inline Vector2D operator / ( float a, const Vector2D& v )
{
	return Vector2D ( a / v.x, a / v.y );
}

inline float operator & ( const Vector2D& u, const Vector2D& v )
{
	return u.x*v.x + u.y*v.y;
}

inline	Vector2D lerp ( const Vector2D& a, const Vector2D& b, float t )
{
	return a + t * (b - a);
}

#endif
