#pragma once
#include"ThreadPool.h"
#include"ThreadPool2.h"
#include<vector>
#include<functional>
#include<math.h>


class ITaskParallel
{
public:
	ITaskParallel()
	{
		if (!threadPool.IsInitialized())
		{
			threadPool.Initialize(threadNums);
		}
	}
	virtual ~ITaskParallel()=0
	{
		if (myHandle >= 0)
		{
			if(myHandle<scheduleList.size()-1)
				scheduleList[myHandle] = NULL;
			scheduleList.erase(scheduleList.end()-1);
		}
	}
	void Dispatch()
	{
		if (pathHandle >= 0 && scheduleList.size() > pathHandle)
		{
			if (scheduleList[pathHandle])
				scheduleList[pathHandle]->Dispatch();
		}
		auto task = [&, this](int _index)
		{
			for (int i = 0; i < this->batch; i++)
				if (!this->Excute(_index + i))
					return;
		};
		threadPool.SetTaskFunction(task);
		for (int i = 0; i < length; i += batch)
		{
			threadPool.AddTask(i);
		}
		threadPool.WaitForAllThread();
	}
	int Schedule(int _length,int _batch,int _pathHandle=-1)
	{
		length = _length;
		batch = _batch;
		pathHandle = _pathHandle;
		for (int i = 0; i<scheduleList.size(); i++)
			if (scheduleList[i] == NULL)
			{
				myHandle = i;
				scheduleList[i] = this;
				return myHandle;
			}
		scheduleList.push_back(this);
		myHandle = scheduleList.size() - 1;
		return myHandle;
	}
protected:
	virtual bool Excute(int index)=0;
private:
	int pathHandle = -1;
	int myHandle = -1;
	int length =1;
	int batch = 1;
private:
	static ThreadPool2<int> threadPool;
	static std::vector<ITaskParallel*> scheduleList;
	static std::vector<std::thread> workers;
public:
	const static int threadNums = 16;
};

