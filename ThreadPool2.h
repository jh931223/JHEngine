#pragma once
#include<vector>
#include<list>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
#include<atomic>

#define USE_COMPETITION_QUEUE
//#define USE_ATOMIC_QUEUE

template<typename Task> class ThreadPool2
{
public:
	ThreadPool2()
	{

	}
	ThreadPool2(int threadNums)
	{
		Initialize(threadNums);
	}
	~ThreadPool2()
	{
		for (int i = 0; i < maxThreadNums; i++)
		{
			workerThreads[i].join();
			delete workerConditions[i];
			delete workerTasks[i];
		}
	}
	bool IsInitialized()
	{
		return isInit;
	}
	void Initialize(int threadNums)
	{
		isInit = true;
		maxThreadNums = threadNums;
		for (int i = 0; i < maxThreadNums; i++)
		{

			workerConditions.push_back(new std::condition_variable);
			workerTasks.push_back(new std::list<Task>);
			workerFlags.push_back(false);
			int num = i;
			waitingThreads.push_back(i);
			workerThreads.push_back(std::thread([=]() { this->Excute(num); }));
		}
	}
	void SetTaskFunction(std::function<void(Task)> _taskFunc)
	{
		taskFunc = _taskFunc;
	}
	void SetFinishCallBack(std::function<void(int)> _finishCallback)
	{
		finishCallBack = _finishCallback;
	}
	void AddTask(Task _task)
	{
		tasks.push_back(_task);
		//{
		//	std::unique_lock<std::mutex> lock(poolMutex);
		//	if (waitingThreads.size())
		//	{
		//		int id = waitingThreads.front();
		//		ChangeState(id);
		//		workerConditions[id]->notify_one();
		//	}
		//}
	}
	void Run()
	{
#ifdef USE_COMPETITION_QUEUE
		taskIDX=0;
		std::unique_lock<std::mutex> lock(poolMutex);
		while (waitingThreads.size())
		{
			int id = waitingThreads.front();
			ChangeState(id);
			workerConditions[id]->notify_one();
		}
#else
		int max = 0;
		int i = 0;
		while (tasks.size())
		{
			workerTasks[i]->push_back(tasks.front());
			tasks.pop_front();
			if (max <= i)
				max = i;
			i++;
			if (i == maxThreadNums)
				i = 0;
		}
		std::unique_lock<std::mutex> lock(poolMutex);
		for (int j = 0; j <= max; j++)
		{
			ChangeState(j);
			workerConditions[j]->notify_one();
		}
#endif
	}
	void WaitForAllThread()
	{
		Run();
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(poolMutex);
				if (!workingThreads.size())
					return;
			}
		}
	}
private:
	void Excute(int id)
	{
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(poolMutex);
				workerConditions[id]->wait(lock, [&]()->bool {return this->workerFlags[id]; });
			}
			Task task;
			bool isHaveTask = false;
#ifdef USE_COMPETITION_QUEUE
			while (GetTask(task))
			{
				isHaveTask = true;
				taskFunc(task);
			}
#else
			isHaveTask = (workerTasks[id]->size() > 0);
			while (workerTasks[id]->size())
			{
				task = workerTasks[id]->front();
				workerTasks[id]->pop_front();
				taskFunc(task);
			}
#endif
			if (isHaveTask)
				finishCallBack(id);
			{
				std::unique_lock<std::mutex> lock2(poolMutex);
				ChangeState(id, false);
			}
		}
	}
	bool GetTask(Task& task)
	{
#ifdef USE_ATOMIC_QUEUE
		int idx = taskIDX.fetch_add(1);
		//int idx = i.load();
		if (idx < tasks.size())
		{
			Task _task = tasks[idx];
			task = _task;
			return true;
		}
#else
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			if (tasks.size())
			{
				Task _task = tasks.front();
				tasks.pop_front();
				task = _task;
				return true;
			}
		}
#endif // USE_ATOMIC_QUEUE
		return false;
	}
	void ChangeState(int id,bool isWaiting=true)
	{
		if (isWaiting)
		{
			for (auto i = waitingThreads.begin(); i != waitingThreads.end();i++)
				if (*i == id)
				{
					waitingThreads.erase(i);
					break;
				}
			workerFlags[id] = true;
			//workingCount++;
			workingThreads.push_back(id);
		}
		else
		{
			for (auto i = workingThreads.begin(); i != workingThreads.end();i++)
				if (*i == id)
				{
					workingThreads.erase(i);
					break;
				}
			workerFlags[id] = false;
			//workingCount--;
			//if (workingCount == 0)
			//	SetEvent(finishEvent);
			waitingThreads.push_back(id);
			if (!workingThreads.size())
				SetEvent(finishEvent);
		}
	}
	std::vector<std::thread> workerThreads;
#ifdef USE_ATOMIC_QUEUE
	std::vector<Task> tasks;
#else
	std::list<Task> tasks;
#endif

	std::vector<bool> workerFlags;

	std::vector<std::condition_variable*> workerConditions;
	std::vector< std::list<Task>* > workerTasks;

	std::list<int> workingThreads;
	std::list<int> waitingThreads;
	HANDLE finishEvent;

	std::mutex poolMutex;
	std::mutex taskMutex;

	int workingCount;

	std::atomic<int> taskIDX;

	int maxThreadNums;

	bool isInit = false;

	std::function<void(Task)> taskFunc;
	std::function<void(int)> finishCallBack;
};