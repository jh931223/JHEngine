#pragma once
#include<vector>
#include<list>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
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
			delete workerMutex[i];
			delete threadConditions[i];
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
			workerMutex.push_back(new std::mutex());
			workerFlags.push_back(false);
			threadConditions.push_back(new std::condition_variable());
			int num = i;
			workerThreads.push_back(std::thread([&]() { this->Excute(num); }));
			waitingThreads.push_back(i);
		}
	}
	void SetTaskFunction(std::function<void(Task)> _taskFunc)
	{
		taskFunc = _taskFunc;
	}
	void AddTask(Task _task)
	{
		taskMutex.lock();
		tasks.push_back(_task);
		taskMutex.unlock();
		{
			std::unique_lock<std::mutex> lock(poolMutex);
			if (waitingThreads.size())
			{
				int id = waitingThreads.front();
				ChangeState(id);
				threadConditions[id]->notify_one();
			}
		}
	}
	void Run()
	{
		taskCondition.notify_all();
	}
	void WaitForAllThread()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(poolMutex);
			if (!workingThreads.size())
				return;
		}
		/*for (int i = 0; i < maxThreadNums;)
		{51
			if (!(workerMutex[i]->try_lock()))
				continue;
			workerMutex[i]->unlock();
			i++;
		}*/
	}
private:
	void Excute(int id)
	{
		while (true)
		{
			printf("id : %d wait\n", id);
			{
				std::unique_lock<std::mutex> lock(poolMutex);
				threadConditions[id]->wait(lock, [&]()->bool{return this->workerFlags[id]; });
			}
			Task task;
			printf("id : %d start\n", id);
			while (GetWork(task))
			{
				taskFunc(task);
			}
			printf("id : %d finished\n", id);
			{
				std::unique_lock<std::mutex> lock2(poolMutex);
				ChangeState(id, false);
			}
			printf("id : %d change state\n", id);
		}
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
			waitingThreads.push_back(id);
		}
	}
	bool GetWork(Task& _task)
	{
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			if (!tasks.size())
				return false;
			_task = tasks.front();
			tasks.pop_front();
		}
		return true;
	}
	std::vector<std::thread> workerThreads;
	std::vector<HANDLE> workerEvents;
	std::list<Task> tasks;
	std::mutex taskMutex;

	std::vector<std::mutex*> workerMutex;
	std::vector<bool> workerFlags;
	std::vector<std::condition_variable*> threadConditions;

	std::list<int> workingThreads;
	std::list<int> waitingThreads;
	std::mutex poolMutex;

	int maxThreadNums;

	bool isInit = false;

	std::function<void(Task)> taskFunc;
};