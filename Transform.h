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

	//  3D 모델의 이동과 회전은 기준 축에 따라서, 2가지 이동과 회전을 할 수 있다. (1) 월드 축을 기준으로 하는 이동 / 회전 월드 축은 게임 공간(게임 화면)의 중심을 중점으로하는 좌표계이다. (2) 로칼 축을 기준으로 하는 이동 / 회전, 로칼 축은 3D 모델의 중심을 원점으로 하는 좌표 계이다.

		// 월드 좌표계는 게임 진행중 고정되어 있는 반면, 로칼 축은 3D 모델의 이동 / 회전에 따라서 축의 중심과 각 축의 방향이 변하게 됨에 유의 해야 한다.

	void TranslateL(XMFLOAT3 T);  //로칼 축 이동 함수
	void RotateL(XMFLOAT3 R);   //로칼 축 회전 함수

	void TranslateW(XMFLOAT3 T); //월드축 이동 함수
	void RotateW(XMFLOAT3 R);     //월드축 이동 함수

	// 크기 변경 즉, 축소 확대는 항상 로칼 축을 기준으로 크기 변경 된다.
	void Scaling(XMFLOAT3 S);


	//  이동 / 회전 함수 - Translate 와 Rotate는 현재를 기준으로 이동 / 회전을 한다.즉, 프레임간의 변경되는 이동량 과 회전량을 계산해서 이동 회전한다.필요에 따라사는 한번에 원하는 위치 또는 회전을 해야하는 경우가 발생하게 된다.,
	void Position(XMFLOAT3 P);   //지정된 좌표로 바로 이동하는 함수
	void Rotation(XMFLOAT3 R);   //지정된 회전 각도로 바로 회전 하는 함수


	//  이동 / 회전 / 크기 변경을 적용하기 위해서는 m_T, (이동 벡터) m_Q(회전 사원수), m_S(크기 벡터)로 부터 변환 행렬을 생성해야 한다.
	XMMATRIX GetTransformMatrix();  //변환 행렬 반환 함수 - 이동/회전/크기변경을 모두 적용

	XMMATRIX GetTranslateMatrix();   //이동 행렬 반환 함수 - 이동만이 적용됨
	XMMATRIX GetRotateMatrix();       //회전 행렬 반환 함수 - 사원수기반의 회전만 적용됨

	//  회전에서는 특정 위치 / 축을 기준으로 회전하는 경우를 고려할 수 있다.예로 지구가 태양을 중심으로 회전시키는 경우등....

	void RotateAround(XMFLOAT3 point, XMFLOAT3 axis, FLOAT angle);  //특정 위치, 축을 기준으로 회전하는 함수
};