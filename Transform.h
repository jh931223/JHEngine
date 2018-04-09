#pragma once

#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class CTransform
{
private:

	XMVECTOR m_T, m_Q, m_S;

public:

	CTransform();
	CTransform(XMFLOAT3 T, XMFLOAT3 R, XMFLOAT3 S);

	//  3D ���� �̵��� ȸ���� ���� �࿡ ����, 2���� �̵��� ȸ���� �� �� �ִ�. (1) ���� ���� �������� �ϴ� �̵� / ȸ�� ���� ���� ���� ����(���� ȭ��)�� �߽��� ���������ϴ� ��ǥ���̴�. (2) ��Į ���� �������� �ϴ� �̵� / ȸ��, ��Į ���� 3D ���� �߽��� �������� �ϴ� ��ǥ ���̴�.

		// ���� ��ǥ��� ���� ������ �����Ǿ� �ִ� �ݸ�, ��Į ���� 3D ���� �̵� / ȸ���� ���� ���� �߽ɰ� �� ���� ������ ���ϰ� �ʿ� ���� �ؾ� �Ѵ�.

	void TranslateL(XMFLOAT3 T);  //��Į �� �̵� �Լ�
	void RotateL(XMFLOAT3 R);   //��Į �� ȸ�� �Լ�

	void TranslateW(XMFLOAT3 T); //������ �̵� �Լ�
	void RotateW(XMFLOAT3 R);     //������ �̵� �Լ�

	// ũ�� ���� ��, ��� Ȯ��� �׻� ��Į ���� �������� ũ�� ���� �ȴ�.
	void Scaling(XMFLOAT3 S);


	//  �̵� / ȸ�� �Լ� - Translate �� Rotate�� ���縦 �������� �̵� / ȸ���� �Ѵ�.��, �����Ӱ��� ����Ǵ� �̵��� �� ȸ������ ����ؼ� �̵� ȸ���Ѵ�.�ʿ信 ������ �ѹ��� ���ϴ� ��ġ �Ǵ� ȸ���� �ؾ��ϴ� ��찡 �߻��ϰ� �ȴ�.,
	void Position(XMFLOAT3 P);   //������ ��ǥ�� �ٷ� �̵��ϴ� �Լ�
	void Rotation(XMFLOAT3 R);   //������ ȸ�� ������ �ٷ� ȸ�� �ϴ� �Լ�


	//  �̵� / ȸ�� / ũ�� ������ �����ϱ� ���ؼ��� m_T, (�̵� ����) m_Q(ȸ�� �����), m_S(ũ�� ����)�� ���� ��ȯ ����� �����ؾ� �Ѵ�.
	XMMATRIX GetTransformMatrix();  //��ȯ ��� ��ȯ �Լ� - �̵�/ȸ��/ũ�⺯���� ��� ����

	XMMATRIX GetTranslateMatrix();   //�̵� ��� ��ȯ �Լ� - �̵����� �����
	XMMATRIX GetRotateMatrix();       //ȸ�� ��� ��ȯ �Լ� - ���������� ȸ���� �����

	//  ȸ�������� Ư�� ��ġ / ���� �������� ȸ���ϴ� ��츦 ����� �� �ִ�.���� ������ �¾��� �߽����� ȸ����Ű�� ����....

	void RotateAround(XMFLOAT3 point, XMFLOAT3 axis, FLOAT angle);  //Ư�� ��ġ, ���� �������� ȸ���ϴ� �Լ�
};