// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#define _WIN32_WINNT 0x0600				// <== CreateFile2  KERNEL32.dll error 해결

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "DxDefine.h"
#include "GameObject.h"
#include "Component.h"
#include "BehaviourComponent.h"
#include "PhysicsClass.h"
#include "JHVector.h"


// 정의

using namespace JHDev;

inline void DebugMessage(std::string s)
{
#ifdef _DEBUG
	printf("%s", s.c_str());
#endif // DEBUG
}
#define PI 0.0174532925f
inline float Deg2Rad(float deg) { return deg * PI / 180; }
inline float GetDistance(XMFLOAT3 _v1, XMFLOAT3 _v2)
{
	XMVECTOR v1, v2;
	float dis;
	v1 = XMVectorSet(_v1.x, _v1.y, _v1.z, 1);
	v2 = XMVectorSet(_v2.x, _v2.y, _v2.z, 1);
	return XMVectorGetX(XMVector3Length(XMVectorSubtract(v2, v1)));
}