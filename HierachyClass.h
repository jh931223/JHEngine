#pragma once
#include <vector>
#include<string>
#include"Singleton.h"
class GameObject;
class HierarchyClass : public Singleton<HierarchyClass>
{
	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> newGameObjects;

public:
	HierarchyClass();
	virtual ~HierarchyClass();
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
			if (i->name== _name)
				return i;
		}
		return NULL;
	}
	void Update();
	void Setup();
	void Start();
};

inline HierarchyClass* Hierarchy() { return HierarchyClass::GetInstance(); }
