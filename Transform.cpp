#include "stdafx.h"
#include "transform.h"

CTransform::CTransform(GameObject* _root)
{
	gameObject = _root;
	SetPosition(XMFLOAT3(0.0, 0.0, 0.0));
	//[Pitch - x 축을 중심으로 회전, Yaw - y 축을 중심으로 회전, Roll - z 축을 중심으로 회전]
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
	m_T = XMVectorSet(P.x, P.y, P.z, 1.0);  //전달 값을 직접 저장한다.
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

	// 일력된 값이 라디안이 아니므로...라디안으로 변경한다.
	FLOAT rx = XMConvertToRadians(R.x);
	FLOAT ry = XMConvertToRadians(R.y);
	FLOAT rz = XMConvertToRadians(R.z);
	// 입력된 회전량은 월드 축을 기준으로 입력이 되었다.이값을 실제 로칼 축값으로 변경하기 위해서는 3D 모델이 회전값으로 회전 시켜야 한다.즉, 3D이 회전되는 것과 동일하게 3D 모델의 로칼 축이 회전된다.
	XMVECTOR Q = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	m_Q = XMQuaternionMultiply(parent->m_Q, Q); //기존 회전값에 입력 회전값을 더한다.
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

	// 입력된 3차원 좌표는 월드 축을 기준으로 입력이 되었다.이값을 실제 로칼 축값으로 변경하기 위해서는 3D 모델이 회전값으로 회전 시켜야 한다.즉, 3D이 회전되는 것과 동일하게 3D 모델의 로칼 축이 회전된다.
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
	XMVECTOR V = XMVectorSet(T.x, T.y, T.z, 0.0);  //이동량을 바로...기존 죄표에 더한다.

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
	// 3D 모델의 크기가 변경되어서 확대 / 축소 되는 경우, 항상 자신의 중심과 축을 이용하게 된다.그러므로, 크기 변경은 월드 좌표 와 로칼 좌표를 따로 구현하지 않는다..
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