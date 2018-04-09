#include "stdafx.h"
#include "Transform.h"

CTransform::CTransform()
{
	m_T = DirectX::XMVectorSet(0.0, 0.0, 0.0, 1.0);
	//[Pitch - x 축을 중심으로 회전, Yaw - y 축을 중심으로 회전, Roll - z 축을 중심으로 회전]
	m_Q = DirectX::XMQuaternionRotationRollPitchYaw(0.0, 0.0, 0.0);
	m_S = DirectX::XMVectorSet(1.0, 1.0, 1.0, 1.0);
}

CTransform::CTransform(XMFLOAT3 T, XMFLOAT3 R, XMFLOAT3 S)
{
	m_T = DirectX::XMVectorSet(T.x, T.y, T.z, 1.0);

	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);

	m_Q = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	// XMQuaternionRotationRollPitchYaw 함수는 x, y, z축을 중심으로 하는 회전각(라디안값임)을 입력받아서, 사원수 벡터를 반환한다.

	m_S = XMVectorSet(S.x, S.y, S.z, 1.0);
}

void CTransform::Position(XMFLOAT3 P)
{
	m_T = XMVectorSet(P.x, P.y, P.z, 1.0);  //전달 값을 직접 저장한다.
}

void CTransform::Rotation(XMFLOAT3 R)
{
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);

	m_Q = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
}

void CTransform::TranslateL(XMFLOAT3 T)
{
	XMMATRIX M;
	XMVECTOR V;

	// 입력된 3차원 좌표는 월드 축을 기준으로 입력이 되었다.이값을 실제 로칼 축값으로 변경하기 위해서는 3D 모델이 회전값으로 회전 시켜야 한다.즉, 3D이 회전되는 것과 동일하게 3D 모델의 로칼 축이 회전된다.
	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(T.x, T.y, T.z, FLOAT(1.0)), M);

	m_T = XMVectorAdd(m_T, V);
}

void CTransform::TranslateW(XMFLOAT3 T)
{
	XMVECTOR V = XMVectorSet(T.x, T.y, T.z, 0.0);  //이동량을 바로...기존 죄표에 더한다.

	m_T = XMVectorAdd(m_T, V);
}


void CTransform::RotateL(XMFLOAT3 R)
{
	XMMATRIX M;
	XMVECTOR V;

	// 일력된 값이 라디안이 아니므로...라디안으로 변경한다.
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);
	// 입력된 회전량은 월드 축을 기준으로 입력이 되었다.이값을 실제 로칼 축값으로 변경하기 위해서는 3D 모델이 회전값으로 회전 시켜야 한다.즉, 3D이 회전되는 것과 동일하게 3D 모델의 로칼 축이 회전된다.

	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(rx, ry, rz, FLOAT(1.0)), M);
	// 회전을 위한 3차원 벡터에는 회전을 위한 기준 축과 회전 각도를 포함하고 있다.DirectXMath를 이용해서 회전시키기 위해서는 회전 기준축의 방향(즉 방향 벡터 - Normalized)을 구하고, 회전량을 분래해서 사용해야 한다.
	XMVECTOR N = XMVector3Normalize(V);  //회전의 중심 축 방향
	XMVECTOR L = XMVector3Length(V);       //회전량
	// 입력에 오류가 있는 경우 회전의 기준축을 구하지 못하는 경우가 발생할 수 있다.예를 들면, (0, 0, 0)이 입력되는 회전 축을 구할 수 없다.이와 같은 오류를 검사해야 한다.

		if (XMVector3NotEqual(N, XMVectorZero()) == true)
		{
			XMVECTOR Q = XMQuaternionRotationNormal(N, XMVectorGetX(L)); //히전 사원수를 구한다.
			// 사원수의 계산에서 두개의 회전을 더하는 것은 아래와 같이 사원수의 곱셈식으로 표현된다.

			m_Q = XMQuaternionMultiply(m_Q, Q); //기존 회전값에 입력 회전값을 더한다. 
		}
}

void CTransform::RotateW(XMFLOAT3 R)
{
	XMVECTOR V;
	// 일력된 값이 라디안이 아니므로...라디안으로 변경한다.

	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);

	V = XMVectorSet(rx, ry, rz, FLOAT(1.0));
	// 회전을 위한 3차원 벡터에는 회전을 위한 기준 축과 회전 각도를 포함하고 있다.DirectXMath를 이용해서 회전시키기 위해서는 회전 기준축의 방향(즉 방향 벡터 - Normalized)을 구하고, 회전량을 분래해서 사용해야 한다

	XMVECTOR N = XMVector3Normalize(V);
	XMVECTOR L = XMVector3Length(V);
	// 입력에 오류가 있는 경우 회전의 기준축을 구하지 못하는 경우가 발생할 수 있다.예를 들면, (0, 0, 0)이 입력되는 회전 축을 구할 수 없다.이와 같은 오류를 검사해야 한다.

		if (XMVector3NotEqual(N, XMVectorZero()) == true)
		{
			XMVECTOR Q = XMQuaternionRotationNormal(N, XMVectorGetX(L));
			// 사원수의 계산에서 두개의 회전을 더하는 것은 아래와 같이 사원수의 곱셈식으로 표현된다.

			m_Q = XMQuaternionMultiply(m_Q, Q);
		}
}

void CTransform::Scaling(XMFLOAT3 S)
{
	// 3D 모델의 크기가 변경되어서 확대 / 축소 되는 경우, 항상 자신의 중심과 축을 이용하게 된다.그러므로, 크기 변경은 월드 좌표 와 로칼 좌표를 따로 구현하지 않는다..
	m_S = XMVectorSet(S.x, S.y, S.z, 1.0);
}

XMMATRIX CTransform::GetTransformMatrix()
{
	XMMATRIX tM = XMMatrixTranslationFromVector(m_T);
	XMMATRIX rM = XMMatrixRotationQuaternion(m_Q);
	XMMATRIX sM = XMMatrixScalingFromVector(m_S);

	return sM * rM * tM;
}

XMMATRIX CTransform::GetTranslateMatrix()
{
	return XMMatrixTranslationFromVector(m_T);
}

XMMATRIX CTransform::GetRotateMatrix()
{
	return  XMMatrixRotationQuaternion(m_Q);
}
void CTransform::RotateAround(XMFLOAT3 point, XMFLOAT3 axis, FLOAT angle)
{
	angle = XMConvertToRadians(angle);
	XMVECTOR N = XMVector3Normalize(XMVectorSet(axis.x, axis.y, axis.z, FLOAT(1.0)));

	if (XMVector3NotEqual(N, XMVectorZero()) == true)
	{
		XMVECTOR T = XMVectorSet(point.x, point.y, point.z, 0);
		m_T = m_T - T;

		XMVECTOR QV = XMQuaternionRotationNormal(N, angle);
		XMMATRIX QM = XMMatrixRotationQuaternion(QV);

		m_T = XMVector3Transform(m_T, QM);

		m_T = m_T + T;
	}
}