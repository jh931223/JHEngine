#pragma once

class GameObject;

class CTransform
{
private:

	XMVECTOR m_T, m_Q, m_S;
	XMFLOAT3 localPosition;
	XMFLOAT3 localRotation;
	XMFLOAT3 localScale;
public:
	CTransform * parent;
	GameObject* gameObject;
	CTransform(GameObject* _root);
	CTransform(GameObject* _root,XMFLOAT3 T, XMFLOAT3 R, XMFLOAT3 S);


	void TranslateL(XMFLOAT3 T);  //��Į �� �̵� �Լ�
	void RotateL(XMFLOAT3 R);   //��Į �� ȸ�� �Լ�

	void TranslateW(XMFLOAT3 T); //������ �̵� �Լ�
	void RotateW(XMFLOAT3 R);     //������ �̵� �Լ�

	// ũ�� ���� ��, ��� Ȯ��� �׻� ��Į ���� �������� ũ�� ���� �ȴ�.
	void SetScale(XMFLOAT3 S);


	XMFLOAT3 GetWorldPosition();
	XMFLOAT3 GetLocalPosition();
	XMFLOAT3 GetWorldRotation();
	XMFLOAT3 GetLocalRotation();

	void SetPosition(XMFLOAT3 P);
	void SetLocalPosition(XMFLOAT3 P);
	void SetRotation(XMFLOAT3 R);
	void SetLocalRotation(XMFLOAT3 R);


	XMMATRIX GetTransformMatrix();

	XMMATRIX GetTranslateMatrix();
	XMMATRIX GetRotateMatrix();

	XMFLOAT3 forward();
	XMFLOAT3 right();
	XMFLOAT3 up();

	void SetParent(CTransform* _parent);

	void RotateAround(XMFLOAT3 point, XMFLOAT3 axis, FLOAT angle);  //Ư�� ��ġ, ���� �������� ȸ���ϴ� �Լ�
};