#pragma once
#include<vector>
class Component;
class GameObject
{
public:
	XMFLOAT3 position;
	XMFLOAT3 euler;
	XMFLOAT3 scale;
	std::vector<Component*> components;
	std::vector<GameObject*> childObjects;
	std::string name;
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
				return i;
		return NULL;
	}
	void Update();
};

