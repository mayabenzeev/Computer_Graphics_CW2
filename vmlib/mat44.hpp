#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2)

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator () allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m(1,2);
 *    m(0,3) = 3.f;
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator() (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator() (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*( Mat44f const& aLeft, Mat44f const& aRight ) noexcept
{
	Mat44f result = {};

    for( std::size_t i = 0; i < 4; ++i ) // Row index of aLeft
	{  
        for( std::size_t j = 0; j < 4; ++j ) // Column index of aRight
		{  	
			// calculate the (i,j) element with the column index of aLeft and row index of aRight
			result(i, j) = 0;
            for (int k = 0; k < 4; ++k) { 
                result(i,j) += aLeft(i,k) * aRight(k,j);
            }
        }
    }
	return result;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	Vec4f result = {};

    for( std::size_t i = 0; i < 4; ++i ) // Row index of aLeft
	{  
        result[i] = aLeft(i, 0)*aRight.x + 
					aLeft(i, 1)*aRight.y + 
					aLeft(i, 2)*aRight.z + 
					aLeft(i, 3)*aRight.w;
	}
	return result
}

// Functions:

Mat44f invert( Mat44f const& aM ) noexcept;

inline
Mat44f transpose( Mat44f const& aM ) noexcept
{
	Mat44f ret;
	for( std::size_t i = 0; i < 4; ++i )
	{
		for( std::size_t j = 0; j < 4; ++j )
			ret(j,i) = aM(i,j);
	}
	return ret;
}

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	/*
	[1 0 0 0]
	[0 cos(-X Angle) -sin(-X Angle) 0]
	[0 sin(-X Angle) cos(-X Angle) 0]
	[0 0 0 1]
	*/
	float c = std::cos(aAngle);
    float s = std::sin(aAngle);
	return {1.f, 0.f, 0.f, 0.f,
			0.f, c, -s, 0.f,
			0.f, s, c, 0.f,
			0.f, 0.f, 0.f, 1.f};
}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	/*
	[cos(-Y Angle) 0 sin(-Y Angle) 0]
	[0 1 0 0]
	[-sin(-Y Angle) 0 cos(-Y Angle) 0]
	[0 0 0 1]
	*/
	float c = std::cos(aAngle);
    float s = std::sin(aAngle);
	return {c, 0.f, s, 0.f,
			0.f, 1.f, 0.f, 0.f,
			-s, 0.f, c, 0.f,
			0.f, 0.f, 0.f, 1.f};
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	/*
	[cos(-Z Angle) -sin(-Z Angle) 0 0]
	[sin(-Z Angle) cos(-Z Angle) 0 0]
	[0 0 1 0]
	[0 0 0 1] 
	*/
	float c = std::cos(aAngle);
    float s = std::sin(aAngle);
	return {c, -s, 0.f, 0.f,
			s, c, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f};
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	return {
		1.f, 0.f, 0.f, aTranslation.x,
        0.f, 1.f, 0.f, aTranslation.y,
        0.f, 0.f, 1.f, aTranslation.z,
        0.f, 0.f, 0.f, 1.f };
}
inline
Mat44f make_scaling( float aSX, float aSY, float aSZ ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	(void)aSX;  // Avoid warnings about unused arguments until the function
	(void)aSY;  // is properly implemented.
	(void)aSZ;
	return kIdentity44f;
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	// aFovInRadians - angle that defines how much of our scene can we see 
	// aAspect - proportional relationship between a screen´s width and height.
	// aNear - screen
	// aFar - depth limit
	float t = std::tan(aFovInRadians / 2);
	
	return { aAspect / t, 0.f, 0.f, 0.f,
			0.f, 1 / t, 0.f, 0.f,
			0.f, 0.f, -(aFar + aNear) / (aFar - aNear), (-2 * aFar * aNear) / (aFar - aNear),
			0.f, 0.f, 1.f, 0.f };
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
