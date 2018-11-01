#pragma once
#include <vector>
#include <list>
#include<string>
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "ResourcesClass.h"

class GameObject;
class SceneClass
{
	std::list<GameObject*> gameObjects;
	std::list<GameObject*> newGameObjects;

public:
	SceneClass();
	virtual ~SceneClass()=0;
	void AddGameObject(GameObject* _gameObject);
	void DestroyGameObject(GameObject* _gameObject);
	template<class T> GameObject* FindGameObjectWithComponent()
	{
		for (auto i : gameObjects)
		{
			if (i->GetComponent<T>())
				return i;
		}
		return NULL;
	}
	GameObject* FindGameObjectWithName(std::string _name)
	{
		for (auto i : gameObjects)
		{
			if (i->name == _name)
				return i;
		}
		return NULL;
	}
	static GameObject* FindGameObjectWithNameInAllScene(std::string _name)
	{
		std::vector<SceneClass*> *sList = SystemClass::GetInstance()->GetSceneList();
		for (int s = 0; s < SystemClass::GetInstance()->GetSceneList()->size(); s++)
		{
			for (auto i : *sList)
			{
				auto result = i->FindGameObjectWithName(_name);
				if (result)
					return result;
			}
		}
		return NULL;
	}
	void Update();
	virtual void Setup()=0;
	void Start();
};

