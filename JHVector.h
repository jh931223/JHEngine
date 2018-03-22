#pragma once
#include <cmath>
namespace JHDev
{
	typedef XMFLOAT3 Vector3;
	typedef bool Bool;
	typedef float Scalar;
	inline Bool operator==(const XMFLOAT3& one,const XMFLOAT3&other) 
	{
		return (one.x == other.x && one.y == other.y && one.z == other.z);
	}

	inline Bool operator!=(const XMFLOAT3& one, const XMFLOAT3&other)
	{
		return one.x != other.x || one.y != other.y || one.z != other.z;
	}

	inline const XMFLOAT3 right()
	{
		return XMFLOAT3(1, 0, 0);
	}

	inline const XMFLOAT3 up()
	{
		return XMFLOAT3(0, 1, 0);
	}

	inline const XMFLOAT3 forward()
	{
		return XMFLOAT3(0, 0, 1);
	}

	inline XMFLOAT3 operator+=(XMFLOAT3& one, const XMFLOAT3&other)
	{
		one = XMFLOAT3(one.x + other.x, one.y + other.y, one.z + other.z);
		return one;
	}
	inline XMFLOAT3 operator-=(XMFLOAT3& one, const XMFLOAT3&other)
	{
		one = XMFLOAT3(one.x - other.x, one.y - other.y, one.z - other.z);
		return one;
	}
	inline XMFLOAT3 operator*=(XMFLOAT3& one, Scalar num)
	{
		one = XMFLOAT3(one.x * num, one.y * num, one.z * num);
		return one;
	}
	inline XMFLOAT3 operator/=(XMFLOAT3& one, Scalar num)
	{
		one = XMFLOAT3(one.x / num, one.y / num, one.z / num);
		return one;
	}

	inline  XMFLOAT3 operator+(const XMFLOAT3& one, const XMFLOAT3&other)
	{
		return XMFLOAT3(one.x + other.x, one.y + other.y, one.z + other.z);
	}
	inline  XMFLOAT3 operator-(const XMFLOAT3& one, const XMFLOAT3&other)
	{
		return XMFLOAT3(one.x - other.x, one.y - other.y, one.z - other.z);
	}

	inline  XMFLOAT3 operator*(const XMFLOAT3& one,Scalar num)
	{
		return XMFLOAT3(one.x * num, one.y * num, one.z * num);
	}

	inline  XMFLOAT3 operator/(const XMFLOAT3& one,Scalar num)
	{
		return XMFLOAT3(one.x / num, one.y / num, one.z / num);
	}
	inline XMFLOAT3 Normalize3(const XMFLOAT3& one)
	{
		if (one.x == 0 && one.y == 0 && one.z == 0)
			return one;
		return one / sqrt(one.x*one.x + one.y*one.y + one.z*one.z);
	}
}

