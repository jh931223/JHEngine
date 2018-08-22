#pragma once
#include <cmath>
namespace JHDev
{
	typedef XMFLOAT3 Vector3;
	typedef bool Bool;
	typedef float Scalar;
	inline Vector3 GetNanVector()
	{
		float nan = sqrt(-1);
		return Vector3(nan, nan, nan);
	}
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
	inline void SetMatrix3X3(XMFLOAT3X3& matrix, XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c)
	{
		matrix._11 = a.x;
		matrix._12 = a.y;
		matrix._13 = a.z;
		matrix._21 = b.x;
		matrix._22 = b.y;
		matrix._23 = b.z;
		matrix._31 = c.x;
		matrix._32 = c.y;
		matrix._33 = c.z;
	}
	inline XMFLOAT3 operator*(const XMFLOAT3& vec, const XMFLOAT3X3& matrix)
	{
		XMMATRIX m = XMLoadFloat3x3(&matrix);
		XMVECTOR v = XMLoadFloat3(&vec);
		XMFLOAT3 store;
		XMStoreFloat3(&store,XMVector3Transform(v, m));
		return store;
	}
	inline XMFLOAT3 operator*(const XMFLOAT3& vec, const XMFLOAT4X4& matrix)
	{
		XMMATRIX m = XMLoadFloat4x4(&matrix);
		XMFLOAT4 newVec(vec.x, vec.y, vec.z, 1);
		XMVECTOR v = XMLoadFloat4(&newVec);
		XMFLOAT4 store;
		XMStoreFloat4(&store, XMVector4Transform(v, m));
		return XMFLOAT3(store.x,store.y,store.z);
	}
	inline XMFLOAT4X4 operator*(const XMFLOAT4X4& _m1, const XMFLOAT4X4& _m2)
	{
		XMMATRIX m1 = XMLoadFloat4x4(&_m1);
		XMMATRIX m2 = XMLoadFloat4x4(&_m2);
		XMFLOAT4X4 store;
		XMStoreFloat4x4(&store, m1*m2);
		return store;
	}
	template<typename T> void swap(T& a, T& b)
	{
		T c;
		c = b;
		b = a;
		a = c;
	}
}

