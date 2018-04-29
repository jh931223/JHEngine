#include "stdafx.h"
#include "transform.h"

CTransform::CTransform(GameObject* _root)
{
	gameObject = _root;
	SetPosition(XMFLOAT3(0.0, 0.0, 0.0));
	//[Pitch - x ���� �߽����� ȸ��, Yaw - y ���� �߽����� ȸ��, Roll - z ���� �߽����� ȸ��]
	SetRotation(XMFLOAT3(0.0, 0.0, 0.0));
	SetScale(XMFLOAT3(1.0, 1.0, 1.0));
}

CTransform::CTransform(GameObject* _root,XMFLOAT3 T, XMFLOAT3 R, XMFLOAT3 S)
{
	gameObject = _root;
	SetPosition(T);
	SetRotation(R);
	SetScale(S);
}

void CTransform::SetPosition(XMFLOAT3 P)
{
	m_T = XMVectorSet(P.x, P.y, P.z, 1.0);  //���� ���� ���� �����Ѵ�.
	if (parent == NULL)
	{
		localPosition = P;
	}
	else
	{
		
		XMVECTOR f = XMVectorMultiply(XMVectorSet(P.x, P.y, P.z, 0),XMQuaternionInverse(parent->m_Q));
		XMStoreFloat3(&localPosition,f);
	}
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
			i->transform->SetLocalPosition(i->transform->GetLocalPosition());
}

void CTransform::SetLocalPosition(XMFLOAT3 P)
{
	if (parent == NULL)
	{
		SetPosition(P);
		return;
	}
	XMMATRIX M;
	XMVECTOR V;
	XMFLOAT3 parentPos = parent->GetWorldPosition();
	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(P.x+ parentPos.x, P.y+ parentPos.y, P.z+ parentPos.z, FLOAT(1.0)), M);
	m_T = V;
	localPosition = P;
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
			i->transform->SetLocalPosition(i->transform->GetLocalPosition());
}

void CTransform::SetRotation(XMFLOAT3 R)
{
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);

	m_Q = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	if (parent == NULL)
	{
		localRotation = R;
	}
	else
	{
		localRotation = R - parent->GetWorldRotation();
	}
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
		{
			i->transform->SetLocalRotation(i->transform->GetLocalRotation());
		}
}

void CTransform::SetLocalRotation(XMFLOAT3 R)
{
	if (parent==NULL)
	{
		SetRotation(R);
		return;
	}
	XMMATRIX M;
	XMVECTOR V;

	// �Ϸµ� ���� ������ �ƴϹǷ�...�������� �����Ѵ�.
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);
	// �Էµ� ȸ������ ���� ���� �������� �Է��� �Ǿ���.�̰��� ���� ��Į �ప���� �����ϱ� ���ؼ��� 3D ���� ȸ�������� ȸ�� ���Ѿ� �Ѵ�.��, 3D�� ȸ���Ǵ� �Ͱ� �����ϰ� 3D ���� ��Į ���� ȸ���ȴ�.
	XMVECTOR Q = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	m_Q = XMQuaternionMultiply(parent->m_Q, Q); //���� ȸ������ �Է� ȸ������ ���Ѵ�.
	localRotation = R;
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
		{
			i->transform->SetLocalRotation(i->transform->GetLocalRotation());
		}
}

void CTransform::TranslateL(XMFLOAT3 T)
{
	XMMATRIX M;
	XMVECTOR V;

	// �Էµ� 3���� ��ǥ�� ���� ���� �������� �Է��� �Ǿ���.�̰��� ���� ��Į �ప���� �����ϱ� ���ؼ��� 3D ���� ȸ�������� ȸ�� ���Ѿ� �Ѵ�.��, 3D�� ȸ���Ǵ� �Ͱ� �����ϰ� 3D ���� ��Į ���� ȸ���ȴ�.
	M = XMMatrixRotationQuaternion(m_Q);
	V = XMVector3Transform(XMVectorSet(T.x, T.y, T.z, FLOAT(1.0)), M);

	m_T = XMVectorAdd(m_T, V);
	localPosition += T;
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
		{
			i->transform->SetLocalPosition(i->transform->GetLocalRotation());
		}
}

void CTransform::TranslateW(XMFLOAT3 T)
{
	XMVECTOR V = XMVectorSet(T.x, T.y, T.z, 0.0);  //�̵����� �ٷ�...���� ��ǥ�� ���Ѵ�.

	m_T = XMVectorAdd(m_T, V);
	if (gameObject->childObjects.size() > 0)
		for (auto i : gameObject->childObjects)
		{
			i->transform->SetLocalPosition(i->transform->GetLocalPosition());
		}
}


void CTransform::RotateL(XMFLOAT3 R)
{
	SetLocalRotation(localRotation + R);
}

void CTransform::RotateW(XMFLOAT3 R)
{
	SetRotation(GetWorldRotation() + R);
}

void CTransform::SetScale(XMFLOAT3 S)
{
	// 3D ���� ũ�Ⱑ ����Ǿ Ȯ�� / ��� �Ǵ� ���, �׻� �ڽ��� �߽ɰ� ���� �̿��ϰ� �ȴ�.�׷��Ƿ�, ũ�� ������ ���� ��ǥ �� ��Į ��ǥ�� ���� �������� �ʴ´�..
	m_S = XMVectorSet(S.x, S.y, S.z, 1.0);
	localScale = S;
}

XMFLOAT3 CTransform::GetWorldPosition()
{
	XMFLOAT3 p;
	XMStoreFloat3(&p, m_T);
	return p;
}

XMFLOAT3 CTransform::GetLocalPosition()
{
	return localPosition;
}

XMFLOAT3 CTransform::GetWorldRotation()
{
	if (parent == NULL)
		return localRotation;
	CTransform* p=parent;
	XMFLOAT3 R=localRotation;
	while (p!=NULL)
	{
		R += p->GetLocalRotation();
		p = p->parent;
	}
	return R;
	//XMVECTOR u, r, f;
	//u = XMVectorSet(0, 1, 0, 0);
	//r = XMVectorSet(1, 0, 0, 0);
	//f = XMVectorSet(0, 0, 1, 0);
	//FLOAT rx, ry, rz;
	//XMQuaternionToAxisAngle(&u, &ry, m_Q);
	//XMQuaternionToAxisAngle(&r, &rx, m_Q);
	//XMQuaternionToAxisAngle(&f, &rz, m_Q);
	//return XMFLOAT3(XMConvertToDegrees(rx), XMConvertToDegrees(ry), XMConvertToDegrees(rz));
}

XMFLOAT3 CTransform::GetLocalRotation()
{
	return localRotation;
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
XMFLOAT3 CTransform::forward()
{
	XMFLOAT3 result;
	XMVECTOR v = XMVectorMultiply(XMVectorSet(0, 0, 1,0),m_Q);
	XMStoreFloat3(&result, v);
	return result;
}
XMFLOAT3 CTransform::right()
{
	XMFLOAT3 result;
	XMVECTOR v = XMVectorMultiply(XMVectorSet(1, 0, 0, 0), m_Q);
	XMStoreFloat3(&result, v);
	return result;
}
XMFLOAT3 CTransform::up()
{
	XMFLOAT3 result;
	XMVECTOR v = XMVectorMultiply(XMVectorSet(0, 1, 0, 0), m_Q);
	XMStoreFloat3(&result, v);
	return result;
}
void CTransform::SetParent(CTransform* _parent)
{
	parent = _parent;
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