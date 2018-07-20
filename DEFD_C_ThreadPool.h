#pragma once
#include "ThreadPool.h"
#include "SystemClass.h"
#include "D3DClass.h"
#include<vector>
#include<list>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
template<typename ResultBuffer> class DEFD_C_ThreadPool
{
public:
	DEFD_C_ThreadPool() {}
	virtual ~DEFD_C_ThreadPool() {}
	void Initialize(int _maxThread, bool _keepOrder = true) override
	{
		maxThreads = _maxThread;
		keepOrder = _keepOrder;
		for (int i = 0; i < _maxThread; i++)
		{
			workerFlag.push_back(false);
			workerTask.push_back(0);
			ResultBuffer b;
			workerResult.push_back(b);
			deferredContexts.push_back(SystemClass::GetInstance()->GetD3D()->CreateDeferredContext());
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
	void AddTask(std::function<ResultBuffer(ID3D11DeviceContext* deviceContext)> _task)
	{
		tasks.push_back(_task);
	}
	void AddTaskFront(std::function<ResultBuffer(ID3D11DeviceContext* deviceContext)> _task)
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
protected:

	void WaitQueueUpdate() override
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
	void RunningQueueUpdate() override
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
	void WorkerFunc(int id) override
	{
		while (true)
		{
			std::unique_lock<std::mutex> lg((*workerLocks[id]));
			workerConditions[id]->wait(lg, [&]()->bool {return workerFlag[id]; });
			ResultBuffer rBuffer = workerTask[id]();
			if (deferredContexts[id]->context)
			{

			}
			workerResult[id] = rBuffer;
			printf("id:%d thread finished task\n", id);
			workerTask[id] = NULL;
			workerFlag[id] = false;
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
public:
	bool isInit = false;
private:
	std::vector<DEFERRED_CONTEXT_BUFFER*> deferredContexts;

	int maxThreads;
	bool keepOrder;
	std::vector<std::thread> workers;
	std::vector<bool> workerFlag;

	std::vector<std::function<ResultBuffer(ID3D11DeviceContext* deviceContext)>> workerTask;
	std::vector<ResultBuffer> workerResult;

	std::list<std::function<ResultBuffer()>> tasks;

	std::list<int> waitQueue;
	std::list<int> runningQueue;
	std::list<int> endQueue;
	std::list<ResultBuffer> resultQueue;

	std::vector<std::mutex*> workerLocks;
	std::vector<std::condition_variable*> workerConditions;
};

