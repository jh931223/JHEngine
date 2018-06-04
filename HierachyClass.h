#pragma once
#include <vector>
#include<string>
#include"Singleton.h"
class GameObject;
class HierachyClass : public Singleton<HierachyClass>
{
	std::vector<GameObject*> gameObjects;

public:
	HierachyClass();
	~HierachyClass();
	void AddGameObject(GameObject* _gameObject);
	void DestroyGameObject(GameObject* _gameObject);
	void Update();
	void Setup();
	void Start();
};

