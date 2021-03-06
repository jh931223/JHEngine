#pragma once
#include<vector>
#include<list>
#include<algorithm>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
template<typename TaskBuffer,typename ResultBuffer> class ThreadPool
{
public:
	ThreadPool() {}

	~ThreadPool() 
	{
		isRunThread = false;
		for (int i = 0; i < maxThreads; i++)
		{
			workerFlags[i] = true;
			workerConditions[i]->notify_one();
		}
		for (int i = 0; i < maxThreads; i++)
		{
			workers[i].join();
		}
	}
	void Initialize(int _maxThread,bool _keepOrder=true)
	{
		maxThreads = _maxThread;
		keepOrder = _keepOrder;
		workerFlags.resize(_maxThread);
		for (int i = 0; i < _maxThread; i++)
		{
			workerConditions.push_back(new std::condition_variable);
			workers.push_back(std::thread([&]() { WorkerFunc(i); }));
			waitingQueue.push_back(i);
		}
		isInit = true;
	}
	void AddTask(TaskBuffer _task)
	{
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			taskQueue.push_back(_task);
			{
				std::unique_lock<std::mutex> lock(workerMutex);
				if (waitingQueue.size())
				{
					int id = waitingQueue.front();
					ChangeState(id);
					workerConditions[id]->notify_one();
				}
			}
		}
	}
	void AddTaskList(std::list<TaskBuffer>& _task)
	{
		if (!_task.size())
			return;
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			for(auto i:_task)
				taskQueue.push_back(i);
			_task.clear();
			{
				std::unique_lock<std::mutex> lock(workerMutex);
				int count= taskQueue.size();
				while (waitingQueue.size()&&count--)
				{
					int id = waitingQueue.front();
					ChangeState(id);
					workerConditions[id]->notify_one();
				}
			}
		}
	}
	void AddTaskList(std::list<TaskBuffer>& _task,int taskQueueLimit)
	{
		if (!_task.size())
			return;
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			
			while(_task.size())
			{
				if (taskQueue.size() >= taskQueueLimit)
					break;
				taskQueue.push_back(_task.front());
				_task.pop_front();
			}
			{
				std::unique_lock<std::mutex> lock(workerMutex);
				int count = taskQueue.size();
				while (waitingQueue.size() && count--)
				{
					int id = waitingQueue.front();
					ChangeState(id);
					workerConditions[id]->notify_one();
				}
			}
		}
	}
	void AddTaskFront(TaskBuffer _task)
	{
		{
			std::unique_lock<std::mutex> lock(taskMutex);
			taskQueue.push_front(_task);
			{
				std::unique_lock<std::mutex> lock(workerMutex);
				if (waitingQueue.size())
				{
					int id = waitingQueue.front();
					ChangeState(id);
					workerConditions[id]->notify_one();
				}
			}
		}
	}
	void ChangeState(int id, bool isWaiting = true)
	{
		if (isWaiting)
		{
			for (auto i = waitingQueue.begin(); i != waitingQueue.end(); i++)
				if (*i == id)
				{
					waitingQueue.erase(i);
					break;
				}
			workerFlags[id] = true;
			workingQueue.push_back(id);
		}
		else
		{
			for (auto i = workingQueue.begin(); i != workingQueue.end(); i++)
				if (*i == id)
				{
					workingQueue.erase(i);
					break;
				}
			workerFlags[id] = false;
			waitingQueue.push_back(id);
		}
	}
	void GetResultQueue(std::list<ResultBuffer>& _resultQueue)
	{
		{
			std::unique_lock<std::mutex> lock(resultMutex);
			for(auto i:resultQueue)
				_resultQueue.push_back(i);
			resultQueue.clear();
		}
	}
	void SetTaskFunc(std::function<ResultBuffer(TaskBuffer)> func)
	{
		taskFunc = func;
	}
	bool IsInitialized()
	{
		return isInit;
	}
protected :

	bool GetTask(TaskBuffer& _task)
	{
		std::unique_lock<std::mutex> lg(taskMutex);
		if (taskQueue.size())
		{
			_task = taskQueue.front();
			taskQueue.pop_front();
			return true;
		}
		return false;
	}
	void ResultEnqueue(ResultBuffer _buffer)
	{
		{
			std::unique_lock<std::mutex> lock(resultMutex);
			resultQueue.push_back(_buffer);
		}
	}
	void WorkerFunc(int id)
	{
		while (isRunThread)
		{
			{
				std::unique_lock<std::mutex> lock(workerMutex);
				workerConditions[id]->wait(lock, [&,this]()->bool {return this->workerFlags[id]; });
			}
			if (!isRunThread)
				break;
			TaskBuffer task;
			while (GetTask(task))
			{
				ResultBuffer rBuffer = taskFunc(task);
				ResultEnqueue(rBuffer);
			}
			{
				std::unique_lock<std::mutex> lock2(workerMutex);
				ChangeState(id, false);
			}
		}
	}
public:
protected:
	bool isInit = false;
	int maxThreads;
	bool keepOrder;
	bool isRunThread = true;
	std::vector<std::thread> workers;
	std::vector<bool> workerFlags;
	std::vector<std::condition_variable*> workerConditions;
	std::mutex workerMutex;

	std::function<ResultBuffer(TaskBuffer)> taskFunc;

	std::list<TaskBuffer> taskQueue;
	std::list<ResultBuffer> resultQueue;
	std::list<int> waitingQueue;
	std::list<int> workingQueue;

	std::mutex taskMutex;
	std::mutex resultMutex;
};

