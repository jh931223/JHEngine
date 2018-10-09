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

	virtual ~ThreadPool() 
	{
		for (int i = 0; i < maxThreads; i++)
		{
			workers[i].join();
		}
	}
	virtual void Initialize(int _maxThread,bool _keepOrder=true)
	{
		maxThreads = _maxThread;
		keepOrder = _keepOrder;
		workerFlag.resize(_maxThread);
		taskOfWorker.resize(_maxThread);
		workerResult.resize(_maxThread);
		for (int i = 0; i < _maxThread; i++)
		{
			workerLocks.push_back(new std::mutex);
			workerConditions.push_back(new std::condition_variable);
			workers.push_back(std::thread([&]() { WorkerFunc(i); }));
			waitQueue.push_back(i);
		}
		isInit = true;
	}
	void AddTask(TaskBuffer _task)
	{
		auto iter = std::find(taskBuffers.begin(), taskBuffers.end(), _task);
		if (iter!=taskBuffers.end())
			return;
		taskBuffers.push_back(_task);
	}
	void AddTaskFront(TaskBuffer _task)
	{

		auto iter = std::find(taskBuffers.begin(), taskBuffers.end(), _task);
		if (iter != taskBuffers.end())
			taskBuffers.erase(iter);
		taskBuffers.push_front(_task);
	}
	void ThreadPoolUpdate()
	{
		if (!isInit)
			return;
		WaitQueueUpdate();
		RunningQueueUpdate();
		EndQueueUpdate();
	}
	std::list<ResultBuffer>* GetResultQueue()
	{
		return &resultQueue;
	}
	bool IsAllTaskFinished()
	{
		if (!IsInitialized())
			return false;
		return !runningQueue.size()&&!taskBuffers.size();
	}
	int GetRunningQueueSize()
	{
		return runningQueue.size();
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
	virtual void WaitQueueUpdate()
	{


		while (waitQueue.size())
		{
			if (!taskBuffers.size())
			{
				break;
			}
			int i = waitQueue.front();
			waitQueue.pop_front();
			taskOfWorker[i] = taskBuffers.front();
			taskBuffers.pop_front();
			runningQueue.push_back(i);
			{
				std::lock_guard<std::mutex> lg(*workerLocks[i]);
				workerFlag[i] = true;
			}
			workerConditions[i]->notify_one();
		}
	}
	virtual void RunningQueueUpdate()
	{
		if (!keepOrder)
		{
			std::list<int>::iterator iter = runningQueue.begin();
			while (iter != runningQueue.end())
			{
				int i = *iter;
				if (workerFlag[i] || !workerLocks[i]->try_lock())
				{
					iter++;
				}
				else
				{
					endQueue.push_back(i);
					runningQueue.erase(iter++);
					workerLocks[i]->unlock();
				}
			}
		}
		else
		{
			while (runningQueue.size())
			{
				int i = runningQueue.front();
				if (workerFlag[i] || !workerLocks[i]->try_lock())
				{
					break;
				}
				endQueue.push_back(i);
				runningQueue.pop_front();
				workerLocks[i]->unlock();
			}
		}
	}
	void EndQueueUpdate()
	{
		while (endQueue.size())
		{
			int i = endQueue.front();
			resultQueue.push_back(workerResult[i]);
			waitQueue.push_back(i);
			endQueue.pop_front();
		}
	}
	virtual TaskBuffer GetTaskBuffer()
	{
		taskMutex.lock();
		TaskBuffer task = taskBuffers.front();
		taskBuffers.pop_front();
		taskMutex.unlock();
		return task;
	}
	virtual TaskBuffer WorkerFunc(int id)
	{
		while (true)
		{
			std::unique_lock<std::mutex> lg( (*workerLocks[id]) );
			workerConditions[id]->wait(lg, [&]()->bool {return workerFlag[id]; });
			ResultBuffer rBuffer = taskFunc(taskOfWorker[id]);
			workerResult[id] = rBuffer;
			//printf("id:%d thread finished task\n", id);
			workerFlag[id] = false;
		}
	}
public:
protected:
	bool isInit = false;
	int maxThreads;
	bool keepOrder;
	std::vector<std::thread> workers;
	std::vector<bool> workerFlag;

	std::vector<TaskBuffer> taskOfWorker;
	std::vector<ResultBuffer> workerResult;

	std::list<TaskBuffer> taskBuffers;

	std::function<ResultBuffer(TaskBuffer)> taskFunc;

	std::list<int> waitQueue;
	std::list<int> runningQueue;
	std::list<int> endQueue;
	std::list<ResultBuffer> resultQueue;

	std::vector<std::mutex*> workerLocks;
	std::vector<std::condition_variable*> workerConditions;

	std::mutex taskMutex;
};

