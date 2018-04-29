#include "stdafx.h"
#include "GameObject.h"
CTransform * Component::transform()
{
	return (gameObject->transform);
}
