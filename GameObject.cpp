#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject()
{
	name = "GameObject";
	Initialize();
	DebugMessage(name + "持失\n");
}

GameObject::GameObject(const std::string& _name):name(_name)
{
	Initialize();
	DebugMessage(name + "持失\n");
}


GameObject::~GameObject()
{
	Initialize();
}

void GameObject::Initialize()
{
	for (auto i : components)
		delete i;
	components.clear();
	for (auto i : childObjects)
		delete i;
	childObjects.clear();
}

bool GameObject::AddChild(GameObject* _childObject, int _index)
{
	if (_index != -1&&(_index >= childObjects.size() || _index < 0))
		return false;
	for (auto i : childObjects)
	{
		if (i == _childObject)
			return false;
	}
	if (_index == -1)
		childObjects.push_back(_childObject);
	else
		childObjects.insert(childObjects.begin() + _index, _childObject);
	return true;
}

void GameObject::OnStart()
{
	for (auto i : components)
		i->OnStart();
}

GameObject * GameObject::GetChild(int _index)
{
	if ((_index >= childObjects.size() || _index < 0))
		return nullptr;
	return childObjects[_index];
}

bool GameObject::AddComponent(Component* _newComponent)
{
	for (auto i : components)
	{
		if (i == _newComponent)
			return false;
	}
	components.push_back(_newComponent);
	_newComponent->gameObject = this;
	return true;
}

void GameObject::Update()
{
	for (auto i : components)
		i->Update();
	for (auto i : childObjects)
		i->Update();
}

