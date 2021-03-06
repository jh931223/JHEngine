#include "stdafx.h"
#include "GameObject.h"
#include "SceneClass.h"
#include "SystemClass.h"

GameObject::GameObject()
{
	name = "GameObject";
	Initialize();
	//DebugMessage(name + "����\n");
}

GameObject::GameObject(const std::string& _name,SceneClass* _scene):name(_name)
{
	Initialize(_scene);
	//DebugMessage(name + "����\n");
}


GameObject::~GameObject()
{
	printf("%s delete\n", name.c_str());
	for (auto i : components)
		delete i;
	components.clear();
	behaviours.clear();
	for (auto i : childObjects)
		delete i;
	childObjects.clear();
	delete transform;
	transform = 0;
}

void GameObject::Initialize(SceneClass* _scene)
{
	transform = new CTransform(this);

	for (auto i : components)
		delete i;
	components.clear();
	for (auto i : childObjects)
		delete i;
	childObjects.clear();
	if (_scene == NULL)
		_scene = SystemClass::GetInstance()->GetMainScene();
	scene = _scene;
	scene->AddGameObject(this);
}

bool GameObject::AddChild(GameObject* _childObject, int _index)
{
	if (_childObject == 0)
		return false;
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
	_childObject->transform->parent = transform;
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
	if (_newComponent->ComponentType())
		behaviours.push_back((BehaviourComponent*)_newComponent);
	components.push_back(_newComponent);
	_newComponent->gameObject = this;
	return true;
}

void GameObject::Destroy(GameObject * _gameObject)
{
	_gameObject->scene->DestroyGameObject(_gameObject);
}

void GameObject::Update()
{
	if (behaviours.size())
	{
		for (int i=0;i<behaviours.size();i++)
		{
			if (behaviours[i]&& behaviours[i]->enabled)
				behaviours[i]->Update();
		}
		//for (auto i : behaviours)
		//{
		//	if(i&&i->enabled)
		//		i->Update();
		//}
	}
	//for (auto i : childObjects)
	//	i->Update();
}

void GameObject::LateUpdate()
{
	if (behaviours.size())
	{
		for (auto i : behaviours)
			i->LateUpdate();
	}
}

