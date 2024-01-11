#include"OPTIMYSEDThreadPool.h"
#include "INTERRUPTABLEthread.h  "
#include<iostream>
#include<functional>
#include<future>
#include<vector>

typedef function<void()>task_type;
typedef void(*FuncType)(int, int);
typedef future<void> res_futur_type;

OPTIMYSEDThreadPool::OPTIMYSEDThreadPool() :m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4), m_thread_queues(m_thread_count)
{}

OPTIMYSEDThreadPool::~OPTIMYSEDThreadPool()
{
	for (auto& t : m_threads) { delete t; }
}

void OPTIMYSEDThreadPool::start()
{
	for (int i = 0; i < m_thread_count; i++)
	{
		m_threads.push_back(thread(new InterruptableThread(this, i)));    //корректировка: замена старой конструкции на новую
	}
}

void OPTIMYSEDThreadPool::stop()
{
	for (int i = 0; i < m_thread_count; i++)
	{
		task_type empty_task;
		m_thread_queues[i].push(empty_task);
	}
	for (auto& t : m_threads) { t->m_threads.join(); }

}

res_futur_type OPTIMYSEDThreadPool::push_task(FuncType f, int id, int arg)
{	int queue_to_push = m_qindex++ % m_thread_count;
	lock_guard<mutex>l(m_locker);
	TWP.task=([=] {f(id, arg); });
	res_futur_type result= TWP.prom.get_future();
	m_thread_queues[queue_to_push].push(TWP);
	return result;
}

void OPTIMYSEDThreadPool::threadFunc(int qindex)
{
	while (true)
	{
		 if (InterruptableThread::checkInterrupted())
		{
			cout << " поток прерван " << endl;
			return;
		}
		
			//затем обработка очередной задачи
		 TaskWithPromise task_to_do;
		bool res;
		int i = 0;
		for (int i = 0; i < m_thread_count; i++)
		{//попытка быстро забрать задачу из очереди
			if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))break;
		}

		if (!res)
		{//вызов блокирующего получения очереди
			m_thread_queues[qindex].pop(task_to_do);
		}
		else if (!task_to_do)
		{//для предотвращения зависания потока кладём обратно задачу-пустышку
			m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
		}
		if (task_to_do.task) 
				{
					task_to_do.task();
					task_to_do.prom.set_value();
				}
	}
}

void OPTIMYSEDThreadPool::interruptPool()
{
	for (InterruptableThread* t : m_threads) { t->interrupt(); }
	for (InterruptableThread* t : m_threads) { t->m_threads.join(); }
	
}

RequestHandler::RequestHandler()
{
	m_tpool.start();
}


RequestHandler::~RequestHandler()
{
	m_tpool.stop();
}

void RequestHandler::pushRequest(FuncType f, int id, int arg)
{
	m_tpool.push_task(f, id, arg);
}

