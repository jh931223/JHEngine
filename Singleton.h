#pragma once

template <typename T> class Singleton
{
public:
	static T * GetInstance()
	{

		static T * ms_Instance = new T;
		//if (ms_Instance == NULL) ms_Instance = new T;
		return ms_Instance;
	};
	static void DestroyInstance()
	{
		delete ms_Instance;
		ms_Instance = NULL;
	};
	
private:
	static T * ms_Instance;
};

template<typename T> T* Singleton<T>::ms_Instance = 0;