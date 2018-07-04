#pragma once
#include<vector>
#include"Transform.h"
class Component;
class GameObject
{
public:
	std::vector<Component*> components;
	std::vector<GameObject*> childObjects;
	std::string name;
	CTransform* transform;
private:
public:
	GameObject();
	GameObject(const std::string& _name);
	~GameObject();
	void Initialize();
	bool AddChild(GameObject* _childObject,int _index =-1);
	virtual void OnStart();
	GameObject* GetChild(int _index);
	bool AddComponent(Component* _newComponent);
	template<typename T> T* GetComponent()
	{
		for (auto i : components)
			if (typeid(*i) == typeid(T))
				return (T*)i;
		return NULL;
	}
	static void Destroy(GameObject* _gameObject);
	void Update();
	void LateUpdate();
};

