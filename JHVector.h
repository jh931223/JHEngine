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
	namespace BasicVector
	{
		const XMFLOAT3 right= XMFLOAT3(1, 0, 0);
		const XMFLOAT3 up = XMFLOAT3(0, 1, 0);
		const XMFLOAT3 forward = XMFLOAT3(0, 0, 1);
	}
	inline const float Dot(const XMFLOAT3& one, const XMFLOAT3& other)
	{
		XMVECTOR vec1 = XMLoadFloat3(&one);
		XMVECTOR vec2 = XMLoadFloat3(&other);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Dot(vec1, vec2));
		return result.x;
	}
	inline const XMFLOAT3 Cross(const XMFLOAT3& one, const XMFLOAT3& other)
	{
		XMVECTOR vec1 = XMLoadFloat3(&one);
		XMVECTOR vec2 = XMLoadFloat3(&other);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Cross(vec1, vec2));
		return result;
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
	inline XMFLOAT2 operator-=(XMFLOAT2& one, const XMFLOAT2&other)
	{
		one = XMFLOAT2(one.x - other.x, one.y - other.y);
		return one;
	}	
	inline XMFLOAT2 operator+=(XMFLOAT2& one, const XMFLOAT2&other)
	{
		one = XMFLOAT2(one.x + other.x, one.y + other.y);
		return one;
	}
	inline XMFLOAT4 operator+=(XMFLOAT4& one, const XMFLOAT4&other)
	{
		one.x = one.x + other.x;
		one.y = one.y + other.y;
		one.z = one.z + other.z;
		one.w=one.w + other.w;
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
	inline  XMINT3 operator+(const XMINT3& one, const XMINT3&other)
	{
		return XMINT3(one.x + other.x, one.y + other.y, one.z + other.z);
	}
	inline  XMFLOAT3 operator-(const XMFLOAT3& one, const XMFLOAT3&other)
	{
		return XMFLOAT3(one.x - other.x, one.y - other.y, one.z - other.z);
	}

	inline  XMFLOAT3 operator*(const XMFLOAT3& one,Scalar num)
	{
		return XMFLOAT3(one.x * num, one.y * num, one.z * num);
	}
	inline  XMFLOAT3 operator*(Scalar num, const XMFLOAT3& one)
	{
		return XMFLOAT3(one.x * num, one.y * num, one.z * num);
	}

	inline  XMFLOAT3 operator/(const XMFLOAT3& one,Scalar num)
	{
		return XMFLOAT3(one.x / num, one.y / num, one.z / num);
	}
	inline  XMFLOAT3 operator/(Scalar num, const XMFLOAT3& one)
	{
		return XMFLOAT3(one.x / num, one.y / num, one.z / num);
	}
	inline XMFLOAT3 Normalize3(const XMFLOAT3& one)
	{
		XMVECTOR vec = XMLoadFloat3(&one);
		vec=XMVector3Normalize(vec);
		XMFLOAT3 result;
		XMStoreFloat3(&result, vec);
		return result;
		/*if (one.x == 0 && one.y == 0 && one.z == 0)
			return one;
		return one / sqrt(one.x*one.x + one.y*one.y + one.z*one.z);*/
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
	inline int SIGN(float x)
	{
		return x >= 0 ? 1 : -1;
	}
	inline float Lerp(float x, float y, float a)
	{
		float d = y - x;
		float f = x + d*a;
		if (d >= 0)
			return (f > x) ? x : f;
		else
			return (f < x) ? x : f;
	}
	inline float Saturate(float x)
	{
		if (x < 0)
			x = 0;
		else if (x > 1)
			x = 1;
		return x;
	}
	inline XMFLOAT3 CalcNormal(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
	{
		XMFLOAT3 f1 = v1 - v2;
		//f1 = Normalize3(f1);
		XMFLOAT3 f2 = v3 - v2;
		//f2 = Normalize3(f2);
		XMVECTOR V1 = XMLoadFloat3(&f1);
		V1 = XMVector3Normalize(V1);
		XMVECTOR V2 = XMLoadFloat3(&f2);
		V2 = XMVector3Normalize(V2);
		XMVECTOR UP = XMVector3Cross(V2, V1);
		UP = XMVector3Normalize(UP);
		XMFLOAT3 n;
		XMStoreFloat3(&n, UP);
		return n;
	}
	inline void QuaternionToEuler(const XMVECTOR &qv1, XMFLOAT3 &euler)
	{
		XMFLOAT4 q1;
		XMStoreFloat4(&q1, qv1);
		double test = q1.x*q1.y + q1.z*q1.w;
		if (test > 0.499f) { // singularity at north pole  
			euler.y = 2 * atan2f(q1.x, q1.w);
			euler.z = XM_PI / 2;
			euler.x = 0;
		}
		else if (test < -0.499f) { // singularity at south pole  
			euler.y = -2 * atan2(q1.x, q1.w);
			euler.z = -XM_PI / 2;
			euler.x = 0;
		}
		else
		{
			double sqx = q1.x*q1.x;
			double sqy = q1.y*q1.y;
			double sqz = q1.z*q1.z;
			euler.y = atan2f(2 * q1.y*q1.w - 2 * q1.x*q1.z, 1 - 2 * sqy - 2 * sqz);
			euler.z = asinf(2 * test);
			euler.x = atan2f(2 * q1.x*q1.w - 2 * q1.y*q1.z, 1 - 2 * sqx - 2 * sqz);
		}
	}
}

