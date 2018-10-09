#pragma once
#include "stdafx.h"
#include "SceneClass.h"

using namespace std;


SceneClass::SceneClass()
{
}


SceneClass::~SceneClass()
{
	for (auto i : gameObjects)
		DestroyGameObject(i);
}

void SceneClass::AddGameObject(GameObject * _gameObject)
{
	gameObjects.push_back(_gameObject);
	newGameObjects.push_back(_gameObject);
}

void SceneClass::DestroyGameObject(GameObject * _gameObject)
{
	delete _gameObject;
	auto g1 = std::find(gameObjects.begin(), gameObjects.end(), _gameObject);
	if (g1 != gameObjects.end())
		gameObjects.erase(g1);
	auto g = std::find(newGameObjects.begin(), newGameObjects.end(), _gameObject);
	if (g != newGameObjects.end())
		newGameObjects.erase(g);
}



void SceneClass::Start()
{
	for (auto iterObj : newGameObjects)
	{
		if(iterObj)
			iterObj->OnStart();
	}
	newGameObjects.clear();
}

void SceneClass::Update()
{
	vector<GameObject*> gObjects = gameObjects;
	vector<GameObject*> newGObjects = newGameObjects;
	newGameObjects.clear();
	if (newGObjects.size())
	{
		for (auto i : newGObjects)
		{
			i->OnStart();
		}
		newGObjects.clear();
	}
	for (auto i : gObjects)
	{
		i->Update();
	}
	for (auto i : gObjects)
	{
		i->LateUpdate();
	}
}