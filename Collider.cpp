#include "stdafx.h"
#include "Collider.h"
#include "PhysicsClass.h"

Collider::Collider()
{
	PhysicsClass::AddCollider(this);
	added = true;
}

Collider::Collider(bool addToList)
{
	if (addToList)
	{
		PhysicsClass::AddCollider(this);
		added = true;
	}
}


Collider::~Collider()
{
	if(added)
		PhysicsClass::RemoveCollider(this);
}
