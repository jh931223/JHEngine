#include "stdafx.h"
#include "Transform.h"

CTransform::CTransform()
{
	m_T = DirectX::XMVectorSet(0.0, 0.0, 0.0, 1.0);
	//[Pitch - x ���� �߽����� ȸ��, Yaw - y ���� �߽����� ȸ��, Roll - z ���� �߽����� ȸ��]
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
	// XMQuaternionRotationRollPitchYaw �Լ��� x, y, z���� �߽����� �ϴ� ȸ����(���Ȱ���)�� �Է¹޾Ƽ�, ����� ���͸� ��ȯ�Ѵ�.

	m_S = XMVectorSet(S.x, S.y, S.z, 1.0);
}

void CTransform::Position(XMFLOAT3 P)
{
	m_T = XMVectorSet(P.x, P.y, P.z, 1.0);  //���� ���� ���� �����Ѵ�.
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

	// �Էµ� 3���� ��ǥ�� ���� ���� �������� �Է��� �Ǿ���.�̰��� ���� ��Į �ప���� �����ϱ� ���ؼ��� 3D ���� ȸ�������� ȸ�� ���Ѿ� �Ѵ�.��, 3D�� ȸ���Ǵ� �Ͱ� �����ϰ� 3D ���� ��Į ���� ȸ���ȴ�.
	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(T.x, T.y, T.z, FLOAT(1.0)), M);

	m_T = XMVectorAdd(m_T, V);
}

void CTransform::TranslateW(XMFLOAT3 T)
{
	XMVECTOR V = XMVectorSet(T.x, T.y, T.z, 0.0);  //�̵����� �ٷ�...���� ��ǥ�� ���Ѵ�.

	m_T = XMVectorAdd(m_T, V);
}


void CTransform::RotateL(XMFLOAT3 R)
{
	XMMATRIX M;
	XMVECTOR V;

	// �Ϸµ� ���� ������ �ƴϹǷ�...�������� �����Ѵ�.
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);
	// �Էµ� ȸ������ ���� ���� �������� �Է��� �Ǿ���.�̰��� ���� ��Į �ప���� �����ϱ� ���ؼ��� 3D ���� ȸ�������� ȸ�� ���Ѿ� �Ѵ�.��, 3D�� ȸ���Ǵ� �Ͱ� �����ϰ� 3D ���� ��Į ���� ȸ���ȴ�.

	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(rx, ry, rz, FLOAT(1.0)), M);
	// ȸ���� ���� 3���� ���Ϳ��� ȸ���� ���� ���� ��� ȸ�� ������ �����ϰ� �ִ�.DirectXMath�� �̿��ؼ� ȸ����Ű�� ���ؼ��� ȸ�� �������� ����(�� ���� ���� - Normalized)�� ���ϰ�, ȸ������ �з��ؼ� ����ؾ� �Ѵ�.
	XMVECTOR N = XMVector3Normalize(V);  //ȸ���� �߽� �� ����
	XMVECTOR L = XMVector3Length(V);       //ȸ����
	// �Է¿� ������ �ִ� ��� ȸ���� �������� ������ ���ϴ� ��찡 �߻��� �� �ִ�.���� ���, (0, 0, 0)�� �ԷµǴ� ȸ�� ���� ���� �� ����.�̿� ���� ������ �˻��ؾ� �Ѵ�.

		if (XMVector3NotEqual(N, XMVectorZero()) == true)
		{
			XMVECTOR Q = XMQuaternionRotationNormal(N, XMVectorGetX(L)); //���� ������� ���Ѵ�.
			// ������� ��꿡�� �ΰ��� ȸ���� ���ϴ� ���� �Ʒ��� ���� ������� ���������� ǥ���ȴ�.

			m_Q = XMQuaternionMultiply(m_Q, Q); //���� ȸ������ �Է� ȸ������ ���Ѵ�. 
		}
}

void CTransform::RotateW(XMFLOAT3 R)
{
	XMVECTOR V;
	// �Ϸµ� ���� ������ �ƴϹǷ�...�������� �����Ѵ�.

	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);

	V = XMVectorSet(rx, ry, rz, FLOAT(1.0));
	// ȸ���� ���� 3���� ���Ϳ��� ȸ���� ���� ���� ��� ȸ�� ������ �����ϰ� �ִ�.DirectXMath�� �̿��ؼ� ȸ����Ű�� ���ؼ��� ȸ�� �������� ����(�� ���� ���� - Normalized)�� ���ϰ�, ȸ������ �з��ؼ� ����ؾ� �Ѵ�

	XMVECTOR N = XMVector3Normalize(V);
	XMVECTOR L = XMVector3Length(V);
	// �Է¿� ������ �ִ� ��� ȸ���� �������� ������ ���ϴ� ��찡 �߻��� �� �ִ�.���� ���, (0, 0, 0)�� �ԷµǴ� ȸ�� ���� ���� �� ����.�̿� ���� ������ �˻��ؾ� �Ѵ�.

		if (XMVector3NotEqual(N, XMVectorZero()) == true)
		{
			XMVECTOR Q = XMQuaternionRotationNormal(N, XMVectorGetX(L));
			// ������� ��꿡�� �ΰ��� ȸ���� ���ϴ� ���� �Ʒ��� ���� ������� ���������� ǥ���ȴ�.

			m_Q = XMQuaternionMultiply(m_Q, Q);
		}
}

void CTransform::Scaling(XMFLOAT3 S)
{
	// 3D ���� ũ�Ⱑ ����Ǿ Ȯ�� / ��� �Ǵ� ���, �׻� �ڽ��� �߽ɰ� ���� �̿��ϰ� �ȴ�.�׷��Ƿ�, ũ�� ������ ���� ��ǥ �� ��Į ��ǥ�� ���� �������� �ʴ´�..
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