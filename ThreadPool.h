#pragma once
#include<vector>
#include<list>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
template<typename ResultBuffer> class ThreadPool
{
public:
	ThreadPool() {}

	~ThreadPool() 
	{
		for (int i = 0; i < maxThreads; i++)
		{
			workers[i].join();
		}
	}
	void Initialize(int _maxThread)
	{
		maxThreads = _maxThread;
		for (int i = 0; i < _maxThread; i++)
		{
			workerFlag.push_back(false);
			workerTask.push_back(0);
			ResultBuffer b;
			workerResult.push_back(b);
		}
		for (int i = 0; i < _maxThread; i++)
		{
			workerLocks.push_back(new std::mutex);
			workerConditions.push_back(new std::condition_variable);
			workers.push_back(std::thread([&]() { WorkerFunc(i); }));
			waitQueue.push_back(i);
		}
		isInit = true;
	}
	void AddTask(std::function<ResultBuffer()> _task)
	{
		tasks.push_back(_task);
	}
	void AddTaskFront(std::function<ResultBuffer()> _task)
	{
		tasks.push_front(_task);
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
private:
	void WaitQueueUpdate()
	{
		while (waitQueue.size())
		{
			if (!tasks.size())
			{
				break;
			}
			int i = waitQueue.front();
			waitQueue.pop_front();
			workerTask[i] = tasks.front();
			tasks.pop_front();
			runningQueue.push_back(i);

			{
				std::lock_guard<std::mutex> lg(*workerLocks[i]);
				workerFlag[i] = true;
			}
			workerConditions[i]->notify_one();
		}
	}
	void RunningQueueUpdate()
	{
		while (runningQueue.size())
		{
			int i = runningQueue.front();
			if (workerFlag[i]|| !workerLocks[i]->try_lock())
			{
				break;
			}
			endQueue.push_back(i);
			runningQueue.pop_front();
			workerLocks[i]->unlock();
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
	void WorkerFunc(int id)
	{
		while (true)
		{
			std::unique_lock<std::mutex> lg( (*workerLocks[id]) );
			workerConditions[id]->wait(lg, [&]()->bool {return workerFlag[id]; });
			ResultBuffer rBuffer = workerTask[id]();
			workerResult[id] = rBuffer;
			printf("id:%d thread finished task\n", id);
			workerTask[id] = NULL;
			workerFlag[id] = false;
		}
	}
public:
	bool isInit = false;
private:
	int maxThreads;
	std::vector<std::thread> workers;
	std::vector<bool> workerFlag;

	std::vector<std::function<ResultBuffer()>> workerTask;
	std::vector<ResultBuffer> workerResult;

	std::list<std::function<ResultBuffer()>> tasks;

	std::list<int> waitQueue;
	std::list<int> runningQueue;
	std::list<int> endQueue;
	std::list<ResultBuffer> resultQueue;

	std::vector<std::mutex*> workerLocks;
	std::vector<std::condition_variable*> workerConditions;
};

